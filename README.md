# AOS Homework 1 — Page Replacement Algorithms and Evaluation

A page-replacement simulator in C. It replays a reference string against
10–100 frames and compares FIFO, MFU, Enhanced Second-Chance (ESC) and our own
Scan-Resistant ESC (SR-ESC) by page faults, interrupts and disk writes.

## Build and run

Requires gcc and GNU make (MSYS2/MinGW on Windows: `mingw32-make`), plus
Python 3 with matplotlib for the figures.

```bash
mingw32-make                 # build pagesim.exe (-O2)
./pagesim.exe --selftest     # known-answer test on the textbook string
mingw32-make run             # results/{random,locality,mine}.csv
mingw32-make plot            # figures/<gen>_<metric>.png (9 figures)
```

Single experiment:

```bash
./pagesim.exe --gen locality --frames 50 --seed 7 --out results/x.csv
```

| option       | meaning                                  | default |
|--------------|------------------------------------------|---------|
| `--gen`      | `random` \| `locality` \| `mine`         | random  |
| `--nrefs`    | number of memory references              | 150000  |
| `--npages`   | page numbers are 1..npages               | 1500    |
| `--frames`   | one frame count, or 0 = 10,20,…,100      | 0       |
| `--pwrite`   | probability that a reference is a write  | 0.3     |
| `--seed`     | RNG seed (same seed → same string)       | 1       |
| `--out`      | CSV output file (default: stdout)        | —       |
| `--selftest` | run the known-answer test and exit       |         |

CSV columns: `gen,algo,frames,faults,interrupts,disk_writes`.

## Layout

```
src/refgen.c     reference string generators (random, locality, mine)
src/sim.c        simulator core; assumptions A1–A6 are documented at the top
src/policy.c     FIFO, ESC, MFU, SR-ESC victim selection and on-hit hooks
src/main.c       CLI parsing, experiment loop, selftest
src/options.c    default options
src/parse_tool.c strtol/strtod-based argument validation
scripts/plot.py  draws the nine figures from the CSV files
```

## Reference strings

* **random** – every reference is uniform over 1..1500.
* **locality** – a sequence of procedure calls; each call lasts a random
  750–1500 references (1/200–1/100 of the string) and touches a random window
  of 20–50 consecutive pages.
* **mine (OLTP + scan)** – models a database buffer pool: OLTP phases whose
  pages follow a Zipf(s = 1) distribution (a few hot pages take most of the
  traffic), alternating with read-only sequential scans of 200–500 consecutive
  pages, i.e. longer than any frame count, so each scan evicts the hot pages
  ("scan pollution").

## Our algorithm: SR-ESC

ESC with one extra class in front: a page whose reference count is still 1
(touched only when it was loaded) is evicted before any page that has been
re-referenced. When the hand clears a reference bit it also demotes that page's
count to 1, so protection decays for pages that stop being used (aging).
On the OLTP + scan string it beats all three benchmarks (at 100 frames:
18 % fewer faults and 31 % fewer disk writes than ESC); on the locality string
it reacts more slowly to a working-set change, which is the known trade-off of
scan-resistant policies (cf. 2Q, InnoDB midpoint insertion).

## Cost model

Each page fault costs one interrupt; evicting a dirty page costs one disk
write plus one more interrupt. See the assumption block in `src/sim.c`.
