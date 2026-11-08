"""Speedup of the multithreaded Game of Life engine vs. number of threads.

Runs `GameOfLife --benchmark` for 1, 2, 4, ... threads (up to the number of
logical cores), computes speedup = T_sequential / T_multithreaded (padded
versions of both) and saves the plot to docs/speedup.png.

Usage (from project/, after building):
    python bench/speedup.py                      # 2000x2000 grid, 200 iterations
    python bench/speedup.py --size 1000 --iters 100 --reps 3
"""
import argparse
import os
import re
import statistics
import subprocess
from pathlib import Path

PROJECT = Path(__file__).resolve().parents[1]
REPO = PROJECT.parent


def run_benchmark(binary, size, iters, threads):
    """Return (t_sequential_padded, t_multithreaded_padded) in seconds."""
    out = subprocess.run(
        [str(binary), "--benchmark", str(size), str(size), str(iters), str(threads)],
        capture_output=True, text=True, check=True,
    ).stdout
    seq = float(re.search(r"Sequential with padding:\s*([\d.eE+-]+)", out).group(1))
    mult = float(re.search(r"Multithreaded with padding:\s*([\d.eE+-]+)", out).group(1))
    return seq, mult


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--size", type=int, default=2000, help="grid width and height")
    parser.add_argument("--iters", type=int, default=200, help="generations per run")
    parser.add_argument("--reps", type=int, default=3, help="runs per thread count (median)")
    parser.add_argument("--binary", default=PROJECT / "build" / "GameOfLife")
    parser.add_argument("--out", default=REPO / "docs" / "speedup.png")
    args = parser.parse_args()

    cores = os.cpu_count() or 1
    threads = sorted({1, cores} | {2 ** i for i in range(1, cores.bit_length()) if 2 ** i <= cores})

    rows = []
    for t in threads:
        runs = [run_benchmark(args.binary, args.size, args.iters, t) for _ in range(args.reps)]
        seq = statistics.median(r[0] for r in runs)
        mult = statistics.median(r[1] for r in runs)
        rows.append((t, seq, mult, seq / mult))
        print(f"threads={t:3d}  seq={seq:8.3f}s  mult={mult:8.3f}s  speedup={seq / mult:5.2f}x")

    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    fig, ax = plt.subplots(figsize=(6, 4))
    ts = [r[0] for r in rows]
    ax.plot(ts, [r[3] for r in rows], "o-", label="Multithreaded (padded)")
    ax.plot(ts, ts, "--", c="grey", label="Ideal (linear)")
    ax.set(
        xlabel="Threads", ylabel="Speedup vs. sequential",
        title=f"Game of Life {args.size}x{args.size}, {args.iters} iterations ({cores} logical cores)",
    )
    ax.set_xticks(ts)
    ax.grid(alpha=0.3)
    ax.legend()
    fig.tight_layout()

    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out, dpi=150)
    print(f"Saved {out}")


if __name__ == "__main__":
    main()
