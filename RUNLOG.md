# RUNLOG

## Baseline A
- Profile: A.json
- Delay: 40 ms
- Miss rate: 7.47%
- Overhead: 1.02x
- Change: None; supplied baseline sender and receiver.
- Why/result: The sender forwards each frame exactly once. Any dropped packet cannot be recovered and becomes a deadline miss.

## Baseline B
- Profile: B.json
- Delay: 40 ms
- Miss rate: 71.27%
- Overhead: 1.02x
- Change: None; supplied baseline sender and receiver.
- Why/result: Profile B has more loss and up to 80 ms delay, so a 40 ms playout deadline causes both dropped and late packets to miss.

## XOR parity experiment
- Profile: A.json
- Delay: 140 ms
- Miss rate: [copy result]
- Overhead: [copy result]
- Change: One XOR parity packet is sent for every pair of frames.
- Why/result: The receiver can reconstruct either one missing frame in a pair while staying below the 2.0x bandwidth cap.

## XOR parity experiment
- Profile: B.json
- Delay: 140 ms
- Miss rate: [copy result]
- Overhead: [copy result]
- Change: Same two-frame XOR parity design.
- Why/result: [fill after test]
