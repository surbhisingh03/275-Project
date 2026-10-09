# Puts all our measurements side by side in one file: results_summary.md (in the 275-Project folder).
#   python3 scripts/summarize_results.py
# No extra packages needed.
#
# Reads (whatever exists):
#   phase1_v2/results/phase1_v2_runs.csv            phase 1, 10 runs
#   phase2/results/phase2_runs.csv                  step 1 (old + new parsing) and step 2, 1/2/4/8 threads
#   phase2/cache_off/results/phase2_runs.csv        the same with MallocLargeCache=0
#   phase2/full_data/results/phase2_runs.csv        step 2 on the full file
#   phase2/full_data/cache_off/results/...          step 2 on the full file, MallocLargeCache=0
#   phase1_v2/results/run_*.txt, phase2/results/*.txt   program output, to check every version found the same trips
#   memory_results/*.csv                            memory-over-time recordings

import csv
import glob
import os
import re

PROJECT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(PROJECT)

THREADS = [1, 2, 4, 8]
NAMES = {
    "phase1": "Phase 1 (objects, 1 thread)",
    "step1_threads_old": "Step 1, phase 1 parsing (timegm, stod)",
    "step1_threads": "Step 1, our parsing (objects)",
    "step2_compact": "Step 2, compact arrays",
}
TIME_COLUMNS = ["time_1h_s", "time_1d_s", "time_1w_s", "time_1m_s"]


def read(path, version=None):
    if not os.path.exists(path):
        return []
    rows = []
    with open(path) as f:
        for r in csv.DictReader(f):
            if version:                 # the phase 1 file has no version/threads columns
                r["version"] = version
                r["threads"] = "1"
            for key in r:
                if key != "version":
                    r[key] = float(r[key])
            rows.append(r)
    return rows


def pick(rows, version, threads):
    return [r for r in rows if r["version"] == version and int(r["threads"]) == threads]


def avg(rows, column):
    values = [r[column] for r in rows]
    return sum(values) / len(values) if values else None


def spread(rows, column):
    values = [r[column] for r in rows]
    return f"{min(values):.2f}–{max(values):.2f}" if values else ""


def compare(base, value):
    """How 'value' compares with 'base' (both times): '3.2× faster', '1.5× slower' or 'about the same'."""
    if not base or not value:
        return "—"
    if abs(value - base) / base < 0.05:
        return "about the same"
    if value < base:
        return f"{base / value:.1f}× faster"
    return f"{value / base:.1f}× slower"


RAM_GB = os.sysconf("SC_PAGE_SIZE") * os.sysconf("SC_PHYS_PAGES") / 1024 ** 3   # this Mac's memory


def fmt(x, digits=2):
    return "—" if x is None else f"{x:.{digits}f}"


phase1 = read("phase1_v2/results/phase1_v2_runs.csv", "phase1")
phase2 = read("phase2/results/phase2_runs.csv")
cache_off = read("phase2/cache_off/results/phase2_runs.csv")
full = read("phase2/full_data/results/phase2_runs.csv")
full_cache_off = read("phase2/full_data/cache_off/results/phase2_runs.csv")
runs = phase1 + phase2
for r in runs + cache_off + full + full_cache_off:
    r["time_all_s"] = sum(r[c] for c in TIME_COLUMNS)

out = []
def line(text=""):
    out.append(text)

line("# Results summary")
line()
if not runs:
    line("No results yet.")
else:
    trips = sorted({int(r["trips"]) for r in runs})
    line(f"Test file: {', '.join(f'{t:,}' for t in trips)} trips. "
         f"Times are averages over all runs (fastest–slowest in brackets where useful).")
    if len(trips) > 1:
        line("**Warning: the runs used different files (different trip counts); do not compare them directly.**")
    line()

    p1 = phase1
    p1_load = avg(p1, "load_s")
    p1_dist = avg(p1, "distance_report_s")
    p1_month = avg(p1, "time_1m_s")

    # ---------- 1. load ----------
    line("## 1. Load time (Signal 1)")
    line()
    line("| Version | Threads | Runs | Load s (avg) | fastest–slowest | vs Phase 1 | vs same version, 1 thread |")
    line("| --- | --- | --- | --- | --- | --- | --- |")
    for version in NAMES:
        for t in THREADS:
            rows = pick(runs, version, t)
            if not rows:
                continue
            load = avg(rows, "load_s")
            one = avg(pick(runs, version, 1), "load_s")
            vs_p1 = "baseline" if version == "phase1" else compare(p1_load, load)
            vs_one = "—" if t == 1 else compare(one, load)
            line(f"| {NAMES[version]} | {t} | {len(rows)} | {load:.2f} | {spread(rows, 'load_s')} | {vs_p1} | {vs_one} |")
    line()

    # ---------- 2. searches ----------
    line("## 2. Search times")
    line()
    line("Distance report = the 5 distance buckets (5 searches). Time windows start 10 March 2014 (1 month = all of March).")
    line()
    line("Cache off = the same program run with MallocLargeCache=0 (see section 3); only step 1 and step 2 were run that way.")
    line()
    line("| Version | Threads | Distance report s | 1 hour s | 1 day s | 1 week s | 1 month s | Report vs Phase 1 | Report s, cache off | 1 month s, cache off |")
    line("| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |")
    for version in NAMES:
        for t in THREADS:
            rows = pick(runs, version, t)
            if not rows:
                continue
            dist = avg(rows, "distance_report_s")
            speed = "baseline" if version == "phase1" else compare(p1_dist, dist)
            off = pick(cache_off, version, t)
            line(f"| {NAMES[version]} | {t} | {dist:.4f} | " +
                 " | ".join(fmt(avg(rows, c), 4) for c in TIME_COLUMNS) + f" | {speed} | "
                 f"{fmt(avg(off, 'distance_report_s'), 4)} | {fmt(avg(off, 'time_1m_s'), 4)} |")
    line()

    # ---------- 3. memory ----------
    line("## 3. Memory (Signal 2 and Signal 3)")
    line()
    line("Trip data = trips × bytes per trip (exact). Peak = whole program (macOS footprint). "
         "Cache off = run with MallocLargeCache=0 (macOS does not keep freed big blocks).")
    line()
    line("| Version | Threads | Trip data MB | Bytes per trip | Peak MB | Peak ÷ data | Peak MB, cache off | Peak ÷ data, cache off |")
    line("| --- | --- | --- | --- | --- | --- | --- | --- |")
    for version in NAMES:
        for t in THREADS:
            rows = pick(runs, version, t)
            if not rows:
                continue
            data = avg(rows, "data_mb")
            peak = avg(rows, "peak_memory_mb")
            per_trip = data * 1024 * 1024 / rows[0]["trips"]
            off = pick(cache_off, version, t)
            off_peak = avg(off, "peak_memory_mb")
            line(f"| {NAMES[version]} | {t} | {data:,.0f} | {per_trip:.0f} | {peak:,.0f} | {peak / data:.2f} | "
                 f"{fmt(off_peak, 0)} | {fmt(off_peak / data if off_peak else None)} |")
    line()

    # ---------- 4. full file ----------
    if full:
        line("## 4. Step 2 on the full file (Signal 2)")
        line()
        f = full
        trips_full = int(f[0]["trips"])
        objects_gb = trips_full * 216 / 1024 ** 3
        line(f"| Measure | Value |")
        line(f"| --- | --- |")
        line(f"| Trips | {trips_full:,} |")
        line(f"| Runs (8 threads) | {len(f)} |")
        line(f"| Load time | {avg(f, 'load_s'):.1f} s ({spread(f, 'load_s')}) |")
        line(f"| Trip data in memory | {avg(f, 'data_mb') / 1024:.2f} GB |")
        line(f"| Peak memory | {avg(f, 'peak_memory_mb') / 1024:.2f} GB |")
        if full_cache_off:
            line(f"| Peak memory, cache off | {avg(full_cache_off, 'peak_memory_mb') / 1024:.2f} GB |")
        fits = "does not fit" if objects_gb > RAM_GB else "fits"
        line(f"| Same trips as Phase 1 objects (216 bytes each) | {objects_gb:.1f} GB: {fits} in this Mac's {RAM_GB:.0f} GB |")
        line(f"| Distance report | {avg(f, 'distance_report_s'):.3f} s |")
        line(f"| 1-month time search | {avg(f, 'time_1m_s'):.4f} s |")
        line()

    # ---------- 5. same answers? ----------
    line("## 5. Did every version find the same trips?")
    line()
    def counts(path):
        with open(path) as fh:
            return re.findall(r"^(.*?): (\d+) trips$", fh.read(), flags=re.M)
    reference_files = sorted(glob.glob("phase1_v2/results/run_*.txt"))
    if reference_files:
        reference = counts(reference_files[0])
        checked, different = 0, []
        for path in sorted(glob.glob("phase2/results/*.txt")):
            checked += 1
            if counts(path) != reference:
                different.append(os.path.basename(path))
        if different:
            line(f"**{len(different)} of {checked} phase 2 runs gave different counts from phase 1:** {', '.join(different[:10])}")
        else:
            line(f"Yes: all {checked} phase 2 runs gave exactly the same counts as phase 1 "
                 f"({len(reference)} searches: 5 distance buckets + 4 time windows).")
    else:
        line("No phase 1 output to compare with.")
    line()

    # ---------- 6. memory over time ----------
    timelines = sorted(glob.glob("memory_results/*.csv"))
    if timelines:
        line("## 6. Memory over time")
        line()
        line("| Recording | Run time s | Peak MB | Chart |")
        line("| --- | --- | --- | --- |")
        for path in timelines:
            with open(path) as fh:
                samples = [(float(r["seconds"]), float(r["mb"])) for r in csv.DictReader(fh)]
            if samples:
                line(f"| {os.path.basename(path)[:-4]} | {samples[-1][0]:.1f} | {max(m for _, m in samples):,.0f} | memory_results/memory_timeline.png |")
        line()

    # ---------- 7. headline numbers ----------
    line("## 7. Headline numbers")
    line()
    old1, old8 = avg(pick(runs, "step1_threads_old", 1), "load_s"), avg(pick(runs, "step1_threads_old", 8), "load_s")
    new1, new8 = avg(pick(runs, "step1_threads", 1), "load_s"), avg(pick(runs, "step1_threads", 8), "load_s")
    if old1 and old8:
        line(f"- **Signal 1, threads with phase 1 parsing:** loading took {old1:.1f} s on 1 thread and "
             f"{old8:.1f} s on 8 threads ({old8 / old1:.1f}× {'slower' if old8 > old1 else 'faster'}).")
    if new1 and new8:
        line(f"- **Signal 1, threads with our parsing:** {new1:.1f} s on 1 thread, {new8:.1f} s on 8 threads "
             f"({new1 / new8:.1f}× faster)" + (f"; {p1_load / new8:.1f}× faster than Phase 1 ({p1_load:.1f} s)." if p1_load else "."))
    s1 = pick(runs, "step1_threads", 1)
    s2 = pick(runs, "step2_compact", 1)
    if s1 and s2:
        line(f"- **Signal 2, memory:** {avg(s1, 'data_mb'):,.0f} MB of trip data as objects vs "
             f"{avg(s2, 'data_mb'):,.0f} MB compact ({avg(s1, 'data_mb') / avg(s2, 'data_mb'):.1f}× smaller).")
    if p1 and s2:
        # compared with Phase 1, not step 1: step 1's normal search times are slowed by Signal 3
        best = min((avg(pick(runs, "step2_compact", t), "distance_report_s"), t) for t in THREADS if pick(runs, "step2_compact", t))
        line(f"- **Signal 2, search speed:** distance report {p1_dist:.3f} s in Phase 1 (objects) vs {avg(s2, 'distance_report_s'):.3f} s compact "
             f"on 1 thread ({p1_dist / avg(s2, 'distance_report_s'):.1f}× faster) and {best[0]:.3f} s at {best[1]} threads ({p1_dist / best[0]:.1f}× faster).")
    if full:
        line(f"- **Signal 2, full file:** all {int(full[0]['trips']):,} trips load in {avg(full, 'data_mb') / 1024:.1f} GB "
             f"compact; as objects they would need {int(full[0]['trips']) * 216 / 1024 ** 3:.1f} GB.")
    s1_on, s1_off = pick(runs, "step1_threads", 1), pick(cache_off, "step1_threads", 1)
    s1_on8, s1_off8 = pick(runs, "step1_threads", 8), pick(cache_off, "step1_threads", 8)
    if s1_on and s1_off and s1_on8 and s1_off8:
        line(f"- **Signal 3, macOS keeps freed memory:** step 1 (8 threads) peaks at {avg(s1_on8, 'peak_memory_mb') / avg(s1_on8, 'data_mb'):.2f}× its trip data "
             f"normally vs {avg(s1_off8, 'peak_memory_mb') / avg(s1_off8, 'data_mb'):.2f}× with MallocLargeCache=0; its distance report (1 thread) takes "
             f"{avg(s1_on, 'distance_report_s'):.2f} s normally vs {avg(s1_off, 'distance_report_s'):.2f} s with the cache off.")
    if full and full_cache_off:
        line(f"- **Signal 3, full file:** step 2 peaks at {avg(full, 'peak_memory_mb') / 1024:.2f} GB normally vs "
             f"{avg(full_cache_off, 'peak_memory_mb') / 1024:.2f} GB with the cache off (trip data {avg(full, 'data_mb') / 1024:.2f} GB).")

with open("results_summary.md", "w") as fh:
    fh.write("\n".join(out) + "\n")
print("\n".join(out))
print("\nSaved results_summary.md")
