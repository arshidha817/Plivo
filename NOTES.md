# Notes

The sender sends every frame immediately and adds one XOR parity packet after every pair of frames.
The receiver stores packets by sequence number, ignores duplicates, and can reconstruct either missing frame in a pair when the parity packet and the other data packet arrive.
This provides loss recovery using only standard C sockets, without a side channel, feedback path, or harness modification.
The wire format uses a type byte, a sequence number or block base, and a 160-byte payload or parity payload.
The measured bandwidth overhead is 1.55x, which is below the 2.0x cap.
Please grade this submission at `delay_ms = 100`.
At 100 ms, profile A and the tested B seeds were all valid; the worst observed miss rate was 0.87%.
At 97 ms, one B run reached exactly the 1.00% cap, while at 96 ms a B run became invalid.
The design cannot recover two or more losses in the same two-frame block, or a loss of both a data packet and its parity packet.
Very large jitter spikes or burst losses beyond the tested profiles can therefore cause deadline misses.
