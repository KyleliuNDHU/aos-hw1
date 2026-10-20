"""Plot page-replacement simulation results.

Usage:
    python scripts/plot.py results/random.csv results/locality.csv results/mine.csv

Each CSV has the header  gen,algo,frames,faults,interrupts,disk_writes
(written by ./pagesim.exe --out ...). For every reference string (gen) and
every metric this script writes one figure, so three CSV files give nine
figures: figures/<gen>_<metric>.png. Each figure has one line per algorithm.
"""
import csv
import sys
from collections import defaultdict
from pathlib import Path

import matplotlib
matplotlib.use("Agg")  # render to files only; no GUI window needed
import matplotlib.pyplot as plt

# (CSV column name, y-axis label)
METRICS = [
    ("faults", "Page faults"),
    ("interrupts", "Interrupts"),
    ("disk_writes", "Disk writes (pages)"),
]


def load(paths):
    """Return data[gen][algo] -> list of CSV rows (dicts)."""
    data = defaultdict(lambda: defaultdict(list))
    for path in paths:
        with open(path, newline="") as f:
            for row in csv.DictReader(f):
                data[row["gen"]][row["algo"]].append(row)
    return data


def plot_one(gen, algos, key, label, outdir):
    """Draw one metric for one reference string, all algorithms on one axes."""
    fig, ax = plt.subplots(figsize=(6.4, 4.2))
    for algo, rows in algos.items():
        rows = sorted(rows, key=lambda r: int(r["frames"]))
        x = [int(r["frames"]) for r in rows]
        y = [int(r[key]) for r in rows]
        ax.plot(x, y, marker="o", label=algo)
    ax.set_xlabel("Number of frames")
    ax.set_ylabel(label)
    ax.set_title(f"{label} vs. frames ({gen} reference string)")
    ax.set_xticks(range(10, 101, 10))
    ax.grid(True, alpha=0.3)
    ax.legend()
    fig.tight_layout()
    path = outdir / f"{gen}_{key}.png"
    fig.savefig(path, dpi=150)
    plt.close(fig)
    return path


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 1
    data = load(argv[1:])
    outdir = Path("figures")
    outdir.mkdir(exist_ok=True)
    for gen, algos in data.items():
        for key, label in METRICS:
            print("wrote", plot_one(gen, algos, key, label, outdir))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
