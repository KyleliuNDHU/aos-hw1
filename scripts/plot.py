"""Plot page-replacement simulation results.

Usage:
    python scripts/plot.py results/random.csv results/locality.csv results/mine.csv

Each CSV must have the header
    gen,algo,frames,faults,interrupts,disk_writes
(exactly what pagesim.exe --out writes). For every reference string (gen)
and every metric this script writes one PNG to figures/<gen>_<metric>.png,
with one line per algorithm. Three CSV files -> nine figures.
"""

import csv                      # standard library: read comma-separated files
import sys                      # standard library: sys.argv = command-line arguments
from collections import defaultdict   # a dict that creates a default value on first use
from pathlib import Path        # object-oriented file paths (works on Windows and Linux)

import matplotlib
matplotlib.use("Agg")           # "Agg" backend draws into files only; no window is opened.
                                # Must be called BEFORE importing pyplot.
import matplotlib.pyplot as plt # pyplot = the plotting API; "plt" is the conventional alias


# Each entry: (CSV column name, text for the y-axis label)
METRICS = [
    	("faults",      "Page faults"),
    	("interrupts",  "Interrupts"),
    	("disk_writes", "Disk writes (pages)"),
	("cost", "Cost (interrupts + disk writes)"),
]


def load(paths):
    """Read every CSV in `paths`.

    Returns a nested dict:  data[gen][algo] -> list of rows,
    where each row is a dict like {"gen": "mine", "algo": "ESC",
    "frames": "10", "faults": "123246", ...}. Values are strings because
    csv.DictReader does not convert types; we convert when plotting.
    """
    data = defaultdict(lambda: defaultdict(list))
    for path in paths:
        with open(path, newline="") as f:          # `with` closes the file automatically
            for row in csv.DictReader(f):           # DictReader uses the header line as keys
                data[row["gen"]][row["algo"]].append(row)
    return data

def value(row, key):
	"""Retrun metric 'key' of one CSV row as int. 'cost' is derived:
	interrupts + disk_writes, the cost definition in the assignment."""
	if key == "cost":
		return int(row["interrupts"]) + int(row["disk_writes"])
	return int(row[key])

def plot_one(gen, algos, key, label, outdir):
    """Draw one figure: metric `key` vs. frames for reference string `gen`.

    `algos` is data[gen]: {"FIFO": [rows...], "ESC": [rows...], ...}.
    One line per algorithm. Returns the path of the PNG written.
    """
    fig, ax = plt.subplots(figsize=(6.4, 4.2))     # fig = the whole canvas, ax = the axes (plot area)
    for algo, rows in algos.items():
        rows = sorted(rows, key=lambda r: int(r["frames"]))   # make sure x is increasing
        x = [int(r["frames"]) for r in rows]        # list comprehension: build a list from rows
        y = [value(r, key) for r in rows]
        ax.plot(x, y, marker="o", label=algo)       # line + circle markers; label feeds the legend
    ax.set_xlabel("Number of frames")
    ax.set_ylabel(label)
    ax.set_title(f"{label} vs. frames ({gen} reference string)")   # f-string: {} is filled in
    ax.set_xticks(range(10, 101, 10))               # ticks exactly at 10, 20, ..., 100
    ax.grid(True, alpha=0.3)                        # light grid lines (alpha = transparency)
    ax.legend()                                     # box showing which colour is which algorithm
    fig.tight_layout()                              # shrink margins so labels are not cut off
    path = outdir / f"{gen}_{key}.png"              # Path objects join with "/"
    fig.savefig(path, dpi=150)                      # 150 dots per inch: sharp enough for a report
    plt.close(fig)                                  # free memory; important when making many figures
    return path


def main(argv):
    if len(argv) < 2:                               # argv[0] is the script name itself
        print(__doc__)                              # __doc__ = the docstring at the top of this file
        return 1
    data = load(argv[1:])                           # argv[1:] = every argument after the script name
    outdir = Path("figures")
    outdir.mkdir(exist_ok=True)                     # create the folder; no error if it exists
    for gen, algos in data.items():
        for key, label in METRICS:
            print("wrote", plot_one(gen, algos, key, label, outdir))
    return 0


if __name__ == "__main__":                          # true only when run as a script, not when imported
    sys.exit(main(sys.argv))                        # exit code 0 = success, like `return 0` in C
