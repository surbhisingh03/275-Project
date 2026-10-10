# Results summary

Test file: 10,238,173 trips. Times are averages over all runs, with the fastest–slowest run where useful.

## 1. Load time (Signal 1)

| Version | Threads | Runs | Load s (avg) | fastest–slowest | vs Phase 1 | vs same version, 1 thread |
| --- | --- | --- | --- | --- | --- | --- |
| Phase 1 (objects, 1 thread) | 1 | 10 | 76.80 | 76.37–77.36 | baseline | — |
| Step 1, phase 1 parsing (timegm, stod) | 1 | 3 | 76.45 | 76.39–76.48 | about the same | — |
| Step 1, phase 1 parsing (timegm, stod) | 2 | 3 | 270.94 | 270.15–271.62 | 3.5× slower | 3.5× slower |
| Step 1, phase 1 parsing (timegm, stod) | 4 | 3 | 305.27 | 303.65–307.47 | 4.0× slower | 4.0× slower |
| Step 1, phase 1 parsing (timegm, stod) | 8 | 3 | 295.65 | 288.75–301.18 | 3.8× slower | 3.9× slower |
| Step 1, our parsing (objects) | 1 | 10 | 16.97 | 16.91–17.11 | 4.5× faster | — |
| Step 1, our parsing (objects) | 2 | 10 | 11.80 | 11.71–12.00 | 6.5× faster | 1.4× faster |
| Step 1, our parsing (objects) | 4 | 10 | 8.12 | 7.94–8.37 | 9.5× faster | 2.1× faster |
| Step 1, our parsing (objects) | 8 | 10 | 6.89 | 6.77–7.03 | 11.2× faster | 2.5× faster |
| Step 2, compact arrays | 1 | 10 | 15.38 | 15.31–15.48 | 5.0× faster | — |
| Step 2, compact arrays | 2 | 10 | 10.24 | 9.82–10.72 | 7.5× faster | 1.5× faster |
| Step 2, compact arrays | 4 | 10 | 6.82 | 6.57–7.27 | 11.3× faster | 2.3× faster |
| Step 2, compact arrays | 8 | 10 | 7.29 | 6.16–9.84 | 10.5× faster | 2.1× faster |

## 2. Search time: the distance report

The distance report = 5 searches, one per distance bucket (0–1, 1–3, 3–5, 5–10, over 10 miles); each scans all trips, counts the matches and adds up their fares, distances, tips and totals.

| Version | Threads | Distance report s (avg) | fastest–slowest | vs Phase 1 |
| --- | --- | --- | --- | --- |
| Phase 1 (objects, 1 thread) | 1 | 1.1101 | 0.66–2.37 | baseline |
| Step 1, phase 1 parsing (timegm, stod) | 1 | 1.5651 | 1.31–1.86 | 1.4× slower |
| Step 1, phase 1 parsing (timegm, stod) | 2 | 0.9529 | 0.76–1.12 | 1.2× faster |
| Step 1, phase 1 parsing (timegm, stod) | 4 | 1.3460 | 0.89–1.85 | 1.2× slower |
| Step 1, phase 1 parsing (timegm, stod) | 8 | 0.8434 | 0.69–1.07 | 1.3× faster |
| Step 1, our parsing (objects) | 1 | 1.7251 | 1.36–2.17 | 1.6× slower |
| Step 1, our parsing (objects) | 2 | 0.9461 | 0.72–1.08 | 1.2× faster |
| Step 1, our parsing (objects) | 4 | 1.3784 | 0.64–2.26 | 1.2× slower |
| Step 1, our parsing (objects) | 8 | 1.0152 | 0.55–2.22 | 1.1× faster |
| Step 2, compact arrays | 1 | 0.1466 | 0.15–0.15 | 7.6× faster |
| Step 2, compact arrays | 2 | 0.0781 | 0.08–0.08 | 14.2× faster |
| Step 2, compact arrays | 4 | 0.0469 | 0.05–0.05 | 23.7× faster |
| Step 2, compact arrays | 8 | 0.0391 | 0.03–0.05 | 28.4× faster |

## 3. Memory (Signal 2)

Trip data = trips × bytes per trip (exact). Peak = whole program (macOS footprint).

| Version | Threads | Trip data MB | Bytes per trip | Peak MB | Peak ÷ data |
| --- | --- | --- | --- | --- | --- |
| Phase 1 (objects, 1 thread) | 1 | 2,109 | 216 | 3,499 | 1.66 |
| Step 1, phase 1 parsing (timegm, stod) | 1 | 2,109 | 216 | 4,222 | 2.00 |
| Step 1, phase 1 parsing (timegm, stod) | 2 | 2,109 | 216 | 4,222 | 2.00 |
| Step 1, phase 1 parsing (timegm, stod) | 4 | 2,109 | 216 | 4,222 | 2.00 |
| Step 1, phase 1 parsing (timegm, stod) | 8 | 2,109 | 216 | 4,222 | 2.00 |
| Step 1, our parsing (objects) | 1 | 2,109 | 216 | 4,222 | 2.00 |
| Step 1, our parsing (objects) | 2 | 2,109 | 216 | 4,222 | 2.00 |
| Step 1, our parsing (objects) | 4 | 2,109 | 216 | 4,222 | 2.00 |
| Step 1, our parsing (objects) | 8 | 2,109 | 216 | 4,223 | 2.00 |
| Step 2, compact arrays | 1 | 557 | 57 | 608 | 1.09 |
| Step 2, compact arrays | 2 | 557 | 57 | 1,117 | 2.01 |
| Step 2, compact arrays | 4 | 557 | 57 | 1,049 | 1.89 |
| Step 2, compact arrays | 8 | 557 | 57 | 929 | 1.67 |

## 4. Step 2 on the full file (Signal 2)

| Measure | Value |
| --- | --- |
| Trips | 71,667,210 |
| Runs (8 threads) | 3 |
| Load time | 49.2 s (46.75–51.27) |
| Trip data in memory | 3.80 GB |
| Peak memory | 5.64 GB |
| Same trips as Phase 1 objects (216 bytes each) | 14.4 GB: does not fit in this Mac's 8 GB |
| Distance report | 0.313 s |

## 5. Did every version find the same trips?

Yes: all 92 phase 2 runs gave exactly the same trip count as phase 1 in each of the 7 distance buckets.

## 6. Memory over time

| Recording | Run time s | Peak MB | Chart |
| --- | --- | --- | --- |
| phase1_objects | 90.9 | 3,396 | memory_results/memory_timeline.png |
| step1_objects_8threads | 10.5 | 4,223 | memory_results/memory_timeline.png |
| step2_compact_8threads | 6.6 | 1,631 | memory_results/memory_timeline.png |

## 7. Headline numbers

- **Signal 1, threads with phase 1 parsing:** loading took 76.4 s on 1 thread and 295.7 s on 8 threads (3.9× slower).
- **Signal 1, threads with our parsing:** 17.0 s on 1 thread, 6.9 s on 8 threads (2.5× faster); 11.2× faster than Phase 1 (76.8 s).
- **Signal 2, memory:** 2,109 MB of trip data as objects vs 557 MB compact (3.8× smaller).
- **Signal 2, search speed:** distance report 1.110 s in Phase 1 (objects) vs 0.147 s compact on 1 thread (7.6× faster) and 0.039 s at 8 threads (28.4× faster).
- **Signal 2, full file:** all 71,667,210 trips load in 3.8 GB compact; as objects they would need 14.4 GB.
