# Results summary

Test file: 10,238,173 trips. Times are averages over all runs, with the fastest–slowest run where useful.

## 1. Load time (Signal 1)

| Version | Threads | Runs | Load s (avg) | fastest–slowest | vs Phase 1 | vs same version, 1 thread |
| --- | --- | --- | --- | --- | --- | --- |
| Phase 1 (objects, 1 thread) | 1 | 10 | 46.47 | 45.98–46.87 | baseline | — |
| Step 1, phase 1 parsing (timegm, stod) | 1 | 3 | 48.42 | 45.81–53.44 | about the same | — |
| Step 1, phase 1 parsing (timegm, stod) | 2 | 3 | 203.57 | 196.12–211.34 | 4.4× slower | 4.2× slower |
| Step 1, phase 1 parsing (timegm, stod) | 4 | 3 | 223.98 | 215.09–230.10 | 4.8× slower | 4.6× slower |
| Step 1, phase 1 parsing (timegm, stod) | 8 | 3 | 203.27 | 193.19–216.50 | 4.4× slower | 4.2× slower |
| Step 1, our parsing (objects) | 1 | 10 | 10.70 | 10.56–11.01 | 4.3× faster | — |
| Step 1, our parsing (objects) | 2 | 10 | 9.64 | 9.45–9.93 | 4.8× faster | 1.1× faster |
| Step 1, our parsing (objects) | 4 | 10 | 8.77 | 8.32–9.06 | 5.3× faster | 1.2× faster |
| Step 1, our parsing (objects) | 8 | 10 | 5.51 | 5.25–5.84 | 8.4× faster | 1.9× faster |
| Step 2, compact arrays | 1 | 10 | 10.78 | 10.67–10.95 | 4.3× faster | — |
| Step 2, compact arrays | 2 | 10 | 9.58 | 9.46–9.81 | 4.9× faster | 1.1× faster |
| Step 2, compact arrays | 4 | 10 | 8.40 | 7.85–8.65 | 5.5× faster | 1.3× faster |
| Step 2, compact arrays | 8 | 10 | 5.22 | 4.84–5.49 | 8.9× faster | 2.1× faster |

## 2. Search time: the distance report

The distance report = 5 searches, one per distance bucket (0–1, 1–3, 3–5, 5–10, over 10 miles); each scans all trips, counts the matches and adds up their fares, distances, tips and totals.

| Version | Threads | Distance report s (avg) | fastest–slowest | vs Phase 1 |
| --- | --- | --- | --- | --- |
| Phase 1 (objects, 1 thread) | 1 | 0.5087 | 0.22–0.60 | baseline |
| Step 1, phase 1 parsing (timegm, stod) | 1 | 0.1665 | 0.17–0.17 | 3.1× faster |
| Step 1, phase 1 parsing (timegm, stod) | 2 | 0.2262 | 0.16–0.34 | 2.2× faster |
| Step 1, phase 1 parsing (timegm, stod) | 4 | 0.3065 | 0.24–0.34 | 1.7× faster |
| Step 1, phase 1 parsing (timegm, stod) | 8 | 0.2653 | 0.25–0.28 | 1.9× faster |
| Step 1, our parsing (objects) | 1 | 0.2062 | 0.17–0.41 | 2.5× faster |
| Step 1, our parsing (objects) | 2 | 0.1818 | 0.14–0.43 | 2.8× faster |
| Step 1, our parsing (objects) | 4 | 0.1600 | 0.15–0.21 | 3.2× faster |
| Step 1, our parsing (objects) | 8 | 0.1210 | 0.09–0.26 | 4.2× faster |
| Step 2, compact arrays | 1 | 0.1021 | 0.10–0.10 | 5.0× faster |
| Step 2, compact arrays | 2 | 0.0546 | 0.05–0.06 | 9.3× faster |
| Step 2, compact arrays | 4 | 0.0318 | 0.03–0.04 | 16.0× faster |
| Step 2, compact arrays | 8 | 0.0212 | 0.02–0.02 | 23.9× faster |

## 2b. Search time: trip lookups by ID

1,000,000 random trip IDs (row numbers, the same in every run), all 18 fields read for each, repeated 5 times. First = round 1, right after the distance report; settled = round 5.

| Version | Threads | Runs | First s (avg) | Settled s (avg) | fastest–slowest settled | ns per lookup | vs Phase 1 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Phase 1 (objects, 1 thread) | 1 | 10 | 0.0239 | 0.0212 | 0.0210–0.0215 | 21 | baseline |
| Step 1, phase 1 parsing (timegm, stod) | 1 | 3 | 0.0217 | 0.0214 | 0.0212–0.0215 | 21 | about the same |
| Step 1, phase 1 parsing (timegm, stod) | 2 | 3 | 0.0220 | 0.0111 | 0.0111–0.0112 | 11 | 1.9× faster |
| Step 1, phase 1 parsing (timegm, stod) | 4 | 3 | 0.0065 | 0.0064 | 0.0063–0.0065 | 6 | 3.3× faster |
| Step 1, phase 1 parsing (timegm, stod) | 8 | 3 | 0.0061 | 0.0058 | 0.0054–0.0066 | 6 | 3.7× faster |
| Step 1, our parsing (objects) | 1 | 10 | 0.0288 | 0.0214 | 0.0211–0.0217 | 21 | about the same |
| Step 1, our parsing (objects) | 2 | 10 | 0.0116 | 0.0111 | 0.0109–0.0114 | 11 | 1.9× faster |
| Step 1, our parsing (objects) | 4 | 10 | 0.0098 | 0.0073 | 0.0068–0.0080 | 7 | 2.9× faster |
| Step 1, our parsing (objects) | 8 | 10 | 0.0203 | 0.0079 | 0.0061–0.0083 | 8 | 2.7× faster |
| Step 2, compact arrays | 1 | 10 | 0.0601 | 0.0599 | 0.0597–0.0609 | 60 | 2.8× slower |
| Step 2, compact arrays | 2 | 10 | 0.0362 | 0.0354 | 0.0346–0.0412 | 35 | 1.7× slower |
| Step 2, compact arrays | 4 | 10 | 0.0272 | 0.0276 | 0.0271–0.0283 | 28 | 1.3× slower |
| Step 2, compact arrays | 8 | 10 | 0.0202 | 0.0216 | 0.0210–0.0221 | 22 | about the same |

## 3. Memory (Signal 2)

Trip data = trips × bytes per trip (exact). Peak = whole program (macOS footprint).

| Version | Threads | Trip data MB | Bytes per trip | Peak MB | Peak ÷ data |
| --- | --- | --- | --- | --- | --- |
| Phase 1 (objects, 1 thread) | 1 | 2,109 | 216 | 5,253 | 2.49 |
| Step 1, phase 1 parsing (timegm, stod) | 1 | 2,109 | 216 | 4,222 | 2.00 |
| Step 1, phase 1 parsing (timegm, stod) | 2 | 2,109 | 216 | 4,222 | 2.00 |
| Step 1, phase 1 parsing (timegm, stod) | 4 | 2,109 | 216 | 4,222 | 2.00 |
| Step 1, phase 1 parsing (timegm, stod) | 8 | 2,109 | 216 | 4,222 | 2.00 |
| Step 1, our parsing (objects) | 1 | 2,109 | 216 | 4,222 | 2.00 |
| Step 1, our parsing (objects) | 2 | 2,109 | 216 | 4,222 | 2.00 |
| Step 1, our parsing (objects) | 4 | 2,109 | 216 | 4,222 | 2.00 |
| Step 1, our parsing (objects) | 8 | 2,109 | 216 | 4,222 | 2.00 |
| Step 2, compact arrays | 1 | 557 | 57 | 608 | 1.09 |
| Step 2, compact arrays | 2 | 557 | 57 | 1,117 | 2.01 |
| Step 2, compact arrays | 4 | 557 | 57 | 1,049 | 1.89 |
| Step 2, compact arrays | 8 | 557 | 57 | 929 | 1.67 |

## 4. Step 2 on the full file (Signal 2)

71,667,210 trips, trip data 3.80 GB in memory. As Phase 1 objects (216 bytes each) the same trips are 14.4 GB (about 29 GB at peak): would not fit comfortably (not tried) in this Mac's 16 GB.

| Threads | Runs | Load s (avg) | fastest–slowest | Distance report s | Trip lookups (1M, settled) s | Peak memory GB |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 3 | 80.1 | 77.3–85.2 | 0.752 | 0.065 | 4.14 |
| 2 | 3 | 85.8 | 83.1–91.2 | 0.512 | 0.047 | 7.62 |
| 4 | 3 | 76.0 | 72.2–80.6 | 0.392 | 0.034 | 7.15 |
| 8 | 3 | 56.6 | 55.8–57.4 | 0.199 | 0.028 | 5.76 |

## 5. Did every version find the same trips?

Yes: all 92 phase 2 runs gave exactly the same trip count as phase 1 in each of the 5 distance buckets, and the same trip lookups (same average total of the looked-up trips).

## 6. Memory over time

| Recording | Run time s | Peak MB | Chart |
| --- | --- | --- | --- |
| phase1_objects | 47.0 | 4,899 | memory_results/memory_timeline.png |
| step1_objects_8threads | 5.2 | 4,230 | memory_results/memory_timeline.png |
| step2_compact_8threads | 4.7 | 931 | memory_results/memory_timeline.png |

## 7. Headline numbers

- **Signal 1, threads with phase 1 parsing:** loading took 48.4 s on 1 thread and 203.3 s on 8 threads (4.2× slower).
- **Signal 1, threads with our parsing:** 10.7 s on 1 thread, 5.5 s on 8 threads (1.9× faster); 8.4× faster than Phase 1 (46.5 s).
- **Signal 2, memory:** 2,109 MB of trip data as objects vs 557 MB compact (3.8× smaller).
- **Signal 2, search speed:** distance report 0.509 s in Phase 1 (objects) vs 0.102 s compact on 1 thread (5.0× faster) and 0.021 s at 8 threads (23.9× faster).
- **Trip lookups (all fields of a few trips):** objects 0.021 s vs compact 0.060 s for 1M lookups on 1 thread: compact is 2.8× slower here.
- **Signal 2, full file:** all 71,667,210 trips load in 3.8 GB compact; as objects they would need 14.4 GB.
