# Watch how much memory a program uses while it runs, and draw it as a line over time.
#
# 1) Record one program (repeat for each version you want to compare):
#      python3 scripts/memory_timeline.py record <name> <command...>
#    e.g.
#      python3 scripts/memory_timeline.py record phase1 phase1_v2/build/taxi_phase1 ~/Downloads/taxi2014/trips_every7th.csv
#      OMP_NUM_THREADS=8 python3 scripts/memory_timeline.py record step2 phase2/build/taxi_compact ~/Downloads/taxi2014/trips_every7th.csv
#    This writes memory_results/<name>.csv (seconds, MB).
#
# 2) Draw every recording on one chart:
#      python3 scripts/memory_timeline.py plot
#    This writes memory_results/memory_timeline.png
#
# Memory = the macOS "footprint" (what Activity Monitor shows in its Memory column), checked
# every 0.1 seconds. It counts memory that macOS has compressed or moved to disk too.
# macOS only. Needs matplotlib for the plot:  python3 -m pip install matplotlib

import csv
import ctypes
import ctypes.util
import os
import subprocess
import sys
import time

FOLDER = "memory_results"
libproc = ctypes.CDLL(ctypes.util.find_library("proc") or "/usr/lib/libSystem.B.dylib")


def footprint_mb(pid):
    """Current memory footprint of process pid, in MB (None if it has ended)."""
    buffer = ctypes.create_string_buffer(512)
    if libproc.proc_pid_rusage(pid, 0, buffer) != 0:
        return None
    # In macOS's rusage_info struct, the footprint is the 8 bytes at position 72
    return int.from_bytes(buffer.raw[72:80], "little") / (1024 * 1024)


def record(name, command):
    os.makedirs(FOLDER, exist_ok=True)
    print("Running:", " ".join(command))
    program = subprocess.Popen(command, stdout=subprocess.DEVNULL)
    start = time.time()
    samples = []
    while program.poll() is None:
        mb = footprint_mb(program.pid)
        if mb is not None:
            samples.append((time.time() - start, mb))
        time.sleep(0.1)

    with open(f"{FOLDER}/{name}.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["seconds", "mb"])
        writer.writerows(samples)
    peak = max(mb for _, mb in samples) if samples else 0
    print(f"Saved {FOLDER}/{name}.csv: {len(samples)} samples, peak {peak:.0f} MB, {samples[-1][0] if samples else 0:.1f} s")


def plot():
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    files = sorted(f for f in os.listdir(FOLDER) if f.endswith(".csv"))
    if not files:
        print("No recordings yet. Use: python3 scripts/memory_timeline.py record <name> <command...>")
        return

    fig, ax = plt.subplots(figsize=(10, 5))
    for file in files:
        seconds, mb = [], []
        with open(f"{FOLDER}/{file}") as f:
            for row in csv.DictReader(f):
                seconds.append(float(row["seconds"]))
                mb.append(float(row["mb"]) / 1024)   # GB
        ax.plot(seconds, mb, label=f"{file[:-4]} (peak {max(mb):.1f} GB)")

    ax.set_title("Memory used while the program runs")
    ax.set_xlabel("seconds since start")
    ax.set_ylabel("memory footprint (GB)")
    ax.grid(alpha=0.3)
    ax.legend()
    fig.tight_layout()
    fig.savefig(f"{FOLDER}/memory_timeline.png", dpi=150)
    print(f"Saved {FOLDER}/memory_timeline.png")


if __name__ == "__main__":
    if len(sys.argv) >= 4 and sys.argv[1] == "record":
        record(sys.argv[2], sys.argv[3:])
    elif len(sys.argv) == 2 and sys.argv[1] == "plot":
        plot()
    else:
        print("Usage:")
        print("  python3 scripts/memory_timeline.py record <name> <command...>")
        print("  python3 scripts/memory_timeline.py plot")
