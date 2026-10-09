# Makes the two charts for the slide from our results files.
#   python3 make_charts.py            (uses the trip count with the most phase 2 runs)
#   python3 make_charts.py 10000000   (only runs that loaded exactly 10,000,000 trips)
# Needs matplotlib once:  python3 -m pip install matplotlib
#
# Reads:  ../phase1_v2/results/phase1_v2_runs.csv  (phase 1, if it exists)
#         results/phase2_runs.csv                   (phase 2)
# Writes: results/signal1_threads.png, results/signal2_data_size.png
#         and prints the averages (with fastest and slowest run) for the report.
#
# Runs on different data files cannot be compared, so only runs with the SAME number
# of trips are used. Runs with other trip counts are listed and left out.

import csv
import os
import sys
import matplotlib
matplotlib.use("Agg")   # save to files, no window
import matplotlib.pyplot as plt

TIME_COLUMNS = ["time_1h_s", "time_1d_s", "time_1w_s", "time_1m_s"]
THREAD_LIST = [1, 2, 4, 8]

# one fixed color per version, so a version has the same color in every chart
COLORS = {
    "phase1": "#52514e",             # gray
    "step1_threads_old": "#eb6834",  # orange
    "step1_threads": "#2a78d6",      # blue
    "step2_compact": "#1baf7a",      # aqua
}
DATA_COLOR = "#2a78d6"   # blue
PEAK_COLOR = "#eb6834"   # orange


def read_runs(path, version_name=None):
    rows = []
    if not os.path.exists(path):
        return rows
    with open(path) as f:
        for r in csv.DictReader(f):
            if version_name:              # the phase 1 file has no version/threads columns
                r["version"] = version_name
                r["threads"] = "1"
            rows.append(r)
    return rows


def average(rows, version, threads, column):
    values = [float(r[column]) for r in rows if r["version"] == version and int(r["threads"]) == threads]
    if not values:
        return None, None, None
    return sum(values) / len(values), min(values), max(values)


phase1_runs = read_runs("../phase1_v2/results/phase1_v2_runs.csv", "phase1")
phase2_runs = read_runs("results/phase2_runs.csv")
all_runs = phase1_runs + phase2_runs
if not all_runs:
    print("No results yet. Run the programs first.")
    raise SystemExit

# ---- choose which trip count to use ----
runs_per_trip_count = {}
for r in all_runs:
    trips = int(r["trips"])
    runs_per_trip_count[trips] = runs_per_trip_count.get(trips, 0) + 1

if len(sys.argv) >= 2:
    trips = int(sys.argv[1])
else:
    # the trip count with the most phase 2 runs (or the most runs at all, if there is no phase 2 yet)
    counts = {}
    for r in (phase2_runs if phase2_runs else all_runs):
        counts[int(r["trips"])] = counts.get(int(r["trips"]), 0) + 1
    trips = max(counts, key=counts.get)

print(f"Using runs with {trips:,} trips.")
for other, n in sorted(runs_per_trip_count.items()):
    if other != trips:
        print(f"  left out: {n} run(s) with {other:,} trips  (python3 make_charts.py {other} to use them)")
print()

runs = [r for r in all_runs if int(r["trips"]) == trips]
if not runs:
    print("No runs with that many trips.")
    raise SystemExit

# all four time searches added together, as one number
for r in runs:
    r["time_all_s"] = sum(float(r[c]) for c in TIME_COLUMNS)

# ---- print a table for the report ----
print(f"{'version':20} {'threads':>7} {'runs':>5} {'load s (avg / min / max)':>30} "
      f"{'distance report s':>18} {'time searches s':>16} {'trip data MB':>13} {'peak MB':>8} {'peak/data':>10}")
for version in ["phase1", "step1_threads_old", "step1_threads", "step2_compact"]:
    for threads in THREAD_LIST:
        n = len([r for r in runs if r["version"] == version and int(r["threads"]) == threads])
        if n == 0:
            continue
        load = average(runs, version, threads, "load_s")
        distance = average(runs, version, threads, "distance_report_s")
        times = average(runs, version, threads, "time_all_s")
        data = average(runs, version, threads, "data_mb")
        memory = average(runs, version, threads, "peak_memory_mb")
        print(f"{version:20} {threads:>7} {n:>5} {load[0]:>12.3f} / {load[1]:.3f} / {load[2]:.3f} "
              f"{distance[0]:>18.4f} {times[0]:>16.4f} {data[0]:>13.0f} {memory[0]:>8.0f} {memory[0] / data[0]:>10.2f}")

os.makedirs("results", exist_ok=True)


def style(ax, title, ylabel):
    ax.set_title(title)
    ax.set_ylabel(ylabel)
    ax.grid(axis="y", alpha=0.3)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)


# ---- Signal 1: what threads did ----
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(10, 4))
for version, label in [("step1_threads_old", "objects, phase 1 parsing (timegm, stod)"),
                       ("step1_threads", "objects, our parsing"),
                       ("step2_compact", "compact arrays, our parsing")]:
    xs, loads, distances = [], [], []
    for t in THREAD_LIST:
        load = average(runs, version, t, "load_s")[0]
        if load is not None:
            xs.append(t)
            loads.append(load)
            distances.append(average(runs, version, t, "distance_report_s")[0])
    if xs:
        ax1.plot(xs, loads, marker="o", markersize=6, linewidth=2, color=COLORS[version], label=label)
        ax2.plot(xs, distances, marker="o", markersize=6, linewidth=2, color=COLORS[version], label=label)

style(ax1, "Load time vs threads", "seconds")
style(ax2, "Distance report (5 searches) vs threads", "seconds")
for ax in (ax1, ax2):
    ax.set_xlabel("threads")
    ax.set_xticks(THREAD_LIST)
    ax.set_ylim(bottom=0)
    if ax.get_lines():   # only if something was drawn
        ax.legend(frameon=False, fontsize=8)
fig.suptitle(f"{trips:,} trips")
fig.tight_layout()
fig.savefig("results/signal1_threads.png", dpi=150)
print("\nsaved results/signal1_threads.png")

# ---- Signal 2: what compact data did (1 thread, so threads are not mixed in) ----
names, data_gb, peak_gb, distance = [], [], [], []
for version, label in [("phase1", "phase 1\nobjects"), ("step1_threads", "step 1\nobjects"), ("step2_compact", "step 2\ncompact")]:
    d = average(runs, version, 1, "data_mb")[0]
    if d is not None:
        names.append(label)
        data_gb.append(d / 1024)
        peak_gb.append(average(runs, version, 1, "peak_memory_mb")[0] / 1024)
        distance.append(average(runs, version, 1, "distance_report_s")[0])

if names:
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(10, 4))

    # left: two bars per version, side by side (trip data, and the peak the whole program reached)
    positions = list(range(len(names)))
    width = 0.38
    data_x = [p - width / 2 for p in positions]
    peak_x = [p + width / 2 for p in positions]
    bars1 = ax1.bar(data_x, data_gb, width * 0.95, color=DATA_COLOR, label="trip data")
    bars2 = ax1.bar(peak_x, peak_gb, width * 0.95, color=PEAK_COLOR, label="peak memory (whole program)")
    ax1.bar_label(bars1, fmt="%.2f", fontsize=8)
    ax1.bar_label(bars2, fmt="%.2f", fontsize=8)
    ax1.set_xticks(positions)
    ax1.set_xticklabels(names)
    ax1.set_ylim(0, max(peak_gb + data_gb) * 1.35)   # room above the bars for the legend
    ax1.legend(frameon=False, fontsize=8, loc="upper right")
    style(ax1, "Memory (1 thread)", "GB")

    # right: distance report time
    bars3 = ax2.bar(names, distance, 0.5, color=DATA_COLOR)
    ax2.bar_label(bars3, fmt="%.3f", fontsize=8)
    style(ax2, "Distance report, 5 searches (1 thread)", "seconds")

    fig.suptitle(f"{trips:,} trips")
    fig.tight_layout()
    fig.savefig("results/signal2_data_size.png", dpi=150)
    print("saved results/signal2_data_size.png")
else:
    print("no 1-thread runs, so results/signal2_data_size.png was not made")
