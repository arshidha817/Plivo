# RUNLOG

All runs used a 30-second duration. The XOR-parity implementation sends each
data frame immediately and sends one XOR parity packet for every pair of frames.
The receiver uses sequence numbers to discard duplicates and recovers exactly one
lost packet from a pair when the parity packet and the other packet arrive.

## Baseline A
- Profile: A.json, seed: 1, delay: 40 ms
- Miss rate: 7.47%; overhead: 1.02x; result: INVALID.
- Change: None; supplied baseline sender and receiver.
- Why/result: The sender forwards every frame once, so dropped packets become deadline misses.

## Baseline B
- Profile: B.json, seed: 1, delay: 40 ms
- Miss rate: 71.27%; overhead: 1.02x; result: INVALID.
- Change: None; supplied baseline sender and receiver.
- Why/result: B has greater loss and up to 80 ms delay, so packets are both lost and late.

## XOR parity: initial valid runs at 140 ms
- A.json, seed 1: 0.20% misses, 1.55x overhead, VALID.
- B.json, seed 1: 0.80% misses, 1.55x overhead, VALID.
- B.json, seed 2: 0.53% misses, 1.55x overhead, VALID.
- B.json, seed 3: 0.47% misses, 1.55x overhead, VALID.
- Change and reason: Added one XOR parity packet per pair. This recovers one lost packet per pair while remaining below the 2.0x overhead limit.

## XOR parity: delay sweep

| Delay | Profile / seed | Miss rate | Overhead | Result | Observation |
|---:|---|---:|---:|---|---|
| 120 ms | A / 1 | 0.20% | 1.55x | VALID | Same reliable recovery as at 140 ms. |
| 120 ms | B / 1 | 0.80% | 1.55x | VALID | |
| 120 ms | B / 2 | 0.53% | 1.55x | VALID | |
| 120 ms | B / 3 | 0.47% | 1.55x | VALID | |
| 110 ms | A / 1 | 0.20% | 1.55x | VALID | Still safely valid. |
| 110 ms | B / 1 | 0.80% | 1.55x | VALID | |
| 110 ms | B / 2 | 0.53% | 1.55x | VALID | |
| 110 ms | B / 3 | 0.47% | 1.55x | VALID | |
| 100 ms | A / 1 | 0.20% | 1.55x | VALID | Lowest delay that was valid across all tested runs with margin. |
| 100 ms | B / 1 | 0.87% | 1.55x | VALID | Worst valid B result at the recommended grading delay. |
| 100 ms | B / 2 | 0.80% | 1.55x | VALID | |
| 100 ms | B / 3 | 0.53% | 1.55x | VALID | |
| 97 ms | A / 1 | 0.20% | 1.55x | VALID | Passes the shown runs but has no safety margin. |
| 97 ms | B / 1 | 0.93% | 1.55x | VALID | |
| 97 ms | B / 2 | 1.00% | 1.55x | VALID | Exactly at the miss-rate cap. |
| 97 ms | B / 3 | 0.60% | 1.55x | VALID | |
| 96 ms | A / 1 | 0.20% | 1.55x | VALID | Not reliable on B. |
| 96 ms | B / 1 | 1.07% | 1.55x | INVALID | Exceeds the miss-rate cap. |
| 96 ms | B / 2 | 1.00% | 1.55x | VALID | Exactly at the cap. |
| 96 ms | B / 3 | 0.53% | 1.55x | VALID | |
| 95 ms | A / 1 | 0.20% | 1.55x | VALID | Not reliable on B. |
| 95 ms | B / 1 | 1.07% | 1.55x | INVALID | |
| 95 ms | B / 2 | 1.07% | 1.55x | INVALID | |
| 95 ms | B / 3 | 0.67% | 1.55x | VALID | |
| 90 ms | A / 1 | 0.20% | 1.55x | VALID | Not reliable on B. |
| 90 ms | B / 1 | 1.47% | 1.55x | INVALID | |
| 90 ms | B / 2 | 1.27% | 1.55x | INVALID | |
| 90 ms | B / 3 | 0.93% | 1.55x | VALID | |
| 80 ms | A / 1 | 0.20% | 1.55x | VALID | The B delay budget is too small. |
| 80 ms | B / 1 | 2.60% | 1.55x | INVALID | |
| 80 ms | B / 2 | 2.80% | 1.55x | INVALID | |
| 80 ms | B / 3 | 2.47% | 1.55x | INVALID | |

## Final choice
- Recommended grading delay: **100 ms**.
- Reason: It is the lowest tested delay that stayed valid for A and all three B seeds, while retaining a small safety margin below the 1.00% miss-rate cap. At 97 ms, one B run reached exactly 1.00%; at 96 ms, B became invalid.
