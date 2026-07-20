#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define FRAME_BYTES 160
#define RAW_FRAME_BYTES 164
#define WIRE_BYTES 165
#define BLOCK_SIZE 2

int main(void) {
    int in_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (in_fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in in_addr = {0};
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(47010);
    in_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(in_fd, (struct sockaddr *)&in_addr, sizeof(in_addr)) < 0) {
        perror("bind 47010");
        return 1;
    }

    int out_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (out_fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in relay = {0};
    relay.sin_family = AF_INET;
    relay.sin_port = htons(47001);
    relay.sin_addr.s_addr = inet_addr("127.0.0.1");

    unsigned char input[RAW_FRAME_BYTES];
    unsigned char wire[WIRE_BYTES];
    unsigned char parity[FRAME_BYTES] = {0};
    uint32_t current_base = UINT32_MAX;

    for (;;) {
        ssize_t n = recvfrom(in_fd, input, sizeof(input), 0, NULL, NULL);
        if (n != RAW_FRAME_BYTES) {
            continue;
        }

        uint32_t seq_be;
        memcpy(&seq_be, input, sizeof(seq_be));
        uint32_t seq = ntohl(seq_be);
        uint32_t base = seq - (seq % BLOCK_SIZE);

        /* Send the original frame immediately. */
        wire[0] = 0; /* data packet */
        memcpy(wire + 1, input, RAW_FRAME_BYTES);
        sendto(out_fd, wire, sizeof(wire), 0,
               (struct sockaddr *)&relay, sizeof(relay));

        /* Build XOR parity for this pair of frames. */
        if (base != current_base) {
            current_base = base;
            memset(parity, 0, sizeof(parity));
        }

        for (int i = 0; i < FRAME_BYTES; i++) {
            parity[i] ^= input[4 + i];
        }

        /* Send parity immediately after the second frame in the pair. */
        if ((seq % BLOCK_SIZE) == BLOCK_SIZE - 1) {
            wire[0] = 1; /* parity packet */
            uint32_t base_be = htonl(base);
            memcpy(wire + 1, &base_be, sizeof(base_be));
            memcpy(wire + 5, parity, FRAME_BYTES);

            sendto(out_fd, wire, sizeof(wire), 0,
                   (struct sockaddr *)&relay, sizeof(relay));
        }
    }
}
