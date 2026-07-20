#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define FRAME_BYTES 160
#define WIRE_BYTES 165
#define BLOCK_SIZE 2
#define SLOT_COUNT 512

struct block_slot {
    int valid;
    uint32_t base;
    int have_data[BLOCK_SIZE];
    unsigned char data[BLOCK_SIZE][FRAME_BYTES];
    int have_parity;
    unsigned char parity[FRAME_BYTES];
};

static struct block_slot slots[SLOT_COUNT];

static void send_frame(int out_fd, const struct sockaddr_in *player,
                       uint32_t seq, const unsigned char *payload) {
    unsigned char output[164];
    uint32_t seq_be = htonl(seq);

    memcpy(output, &seq_be, 4);
    memcpy(output + 4, payload, FRAME_BYTES);

    sendto(out_fd, output, sizeof(output), 0,
           (const struct sockaddr *)player, sizeof(*player));
}

static struct block_slot *get_slot(uint32_t base) {
    struct block_slot *slot = &slots[(base / BLOCK_SIZE) % SLOT_COUNT];

    if (!slot->valid || slot->base != base) {
        memset(slot, 0, sizeof(*slot));
        slot->valid = 1;
        slot->base = base;
    }

    return slot;
}

static void try_recover(int out_fd, const struct sockaddr_in *player,
                        struct block_slot *slot) {
    if (!slot->have_parity) {
        return;
    }

    int missing = -1;
    int missing_count = 0;

    for (int i = 0; i < BLOCK_SIZE; i++) {
        if (!slot->have_data[i]) {
            missing = i;
            missing_count++;
        }
    }

    if (missing_count != 1) {
        return;
    }

    unsigned char recovered[FRAME_BYTES];
    memcpy(recovered, slot->parity, FRAME_BYTES);

    for (int i = 0; i < BLOCK_SIZE; i++) {
        if (i == missing) {
            continue;
        }

        for (int j = 0; j < FRAME_BYTES; j++) {
            recovered[j] ^= slot->data[i][j];
        }
    }

    memcpy(slot->data[missing], recovered, FRAME_BYTES);
    slot->have_data[missing] = 1;

    send_frame(out_fd, player, slot->base + (uint32_t)missing, recovered);
}

int main(void) {
    int in_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (in_fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in in_addr = {0};
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(47002);
    in_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(in_fd, (struct sockaddr *)&in_addr, sizeof(in_addr)) < 0) {
        perror("bind 47002");
        return 1;
    }

    int out_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (out_fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in player = {0};
    player.sin_family = AF_INET;
    player.sin_port = htons(47020);
    player.sin_addr.s_addr = inet_addr("127.0.0.1");

    unsigned char packet[2048];

    for (;;) {
        ssize_t n = recvfrom(in_fd, packet, sizeof(packet), 0, NULL, NULL);
        if (n != WIRE_BYTES) {
            continue;
        }

        if (packet[0] == 0) {
            /* Original data packet: type + seq + 160-byte payload. */
            uint32_t seq_be;
            memcpy(&seq_be, packet + 1, 4);
            uint32_t seq = ntohl(seq_be);

            uint32_t base = seq - (seq % BLOCK_SIZE);
            int index = (int)(seq % BLOCK_SIZE);
            struct block_slot *slot = get_slot(base);

            /* Ignore duplicates after first receipt. */
            if (!slot->have_data[index]) {
                memcpy(slot->data[index], packet + 5, FRAME_BYTES);
                slot->have_data[index] = 1;
                send_frame(out_fd, &player, seq, slot->data[index]);
            }

            try_recover(out_fd, &player, slot);

        } else if (packet[0] == 1) {
            /* Parity packet: type + base sequence + XOR payload. */
            uint32_t base_be;
            memcpy(&base_be, packet + 1, 4);
            uint32_t base = ntohl(base_be);

            struct block_slot *slot = get_slot(base);

            if (!slot->have_parity) {
                memcpy(slot->parity, packet + 5, FRAME_BYTES);
                slot->have_parity = 1;
            }

            try_recover(out_fd, &player, slot);
        }
    }
}
