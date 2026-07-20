# Flaky Network - Reliable UDP Playout

This repository contains a sender and receiver for the Flaky Network systems assignment. A source emits a 160-byte live-media frame every 20 ms; the two programs carry those frames over a lossy, delayed, reordered UDP relay and deliver them to the harness player before each playout deadline.

## Submission deliverables

The required deliverables are present at the repository root:

- `sender.c`, `receiver.c`, and `Makefile`; `make` produces `./sender` and `./receiver`.
- `RUNLOG.md`; one entry or table row per experiment gives profile, `delay_ms`, miss rate, overhead, the change made, and the observed reason/result.
- `NOTES.md`; a concise, ten-sentence maximum description of the design, grading delay, and failure cases.
- `SUMMARY.html`; a coding-agent-created, system-level explanation of the architecture and design choices.

## Design

The implementation uses two-frame XOR forward-error correction (FEC):

1. `sender` receives the harness frame on UDP port 47010 and immediately sends a data packet to the relay on port 47001.
2. After receiving two consecutive frames, `sender` also sends one parity packet containing the XOR of their 160-byte payloads.
3. `receiver` listens on port 47002, stores data and parity by sequence number, and discards duplicates.
4. If exactly one data frame in a pair is missing, `receiver` reconstructs it by XORing the parity packet with the received frame.
5. Recovered and directly received frames are sent to the harness player on port 47020 in the required format: 4-byte big-endian sequence number plus 160-byte payload.

One parity packet per two data packets produces approximately 1.55x measured overhead, below the 2.0x cap. The recommended grading delay is **100 ms**; see `RUNLOG.md` for the full experiment history.

## Build

On Linux, macOS, or WSL:

```bash
make
```

This creates:

```text
./sender
./receiver
```

## Run an experiment

```bash
python3 run.py --profile profiles/A.json --delay_ms 100 --duration 30
python3 run.py --profile profiles/B.json --delay_ms 100 --duration 30
```

Test another deterministic relay seed with:

```bash
python3 run.py --profile profiles/B.json --delay_ms 100 --duration 30 --seed 2
```

The result is valid only when the deadline-miss rate is at most 1.00% and bandwidth overhead is at most 2.0x. Lower valid playout delay is better.

## Files

- `sender.c` - sends data frames and XOR parity packets.
- `receiver.c` - performs deduplication and single-loss recovery per pair.
- `Makefile` - builds `sender` and `receiver`.
- `RUNLOG.md` - measurements and tuning history.
- `NOTES.md` - concise submission notes and grading delay.
- `SUMMARY.html` - detailed system-level architecture summary.

The implementation uses only the C standard/system socket interfaces and does not use a networking framework. The sender and receiver communicate only over the supplied relay media ports; no side channel is used. The provided harness files (`run.py`, `relay.py`, `endpoints.py`, `common.py`, and `score.py`) are not modified.
