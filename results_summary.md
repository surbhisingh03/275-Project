# Results summary

Test file: 10,238,173 trips. Times are averages over all runs, with the fastest–slowest run where useful.

## 1. Load time (Signal 1)

| Version | Threads | Runs | Load s (avg) | fastest–slowest | vs Phase 1 | vs same version, 1 thread |
| --- | --- | --- | --- | --- | --- | --- |
| Phase 1 (objects, 1 thread) | 1 | 10 | 76.78 | 76.08–77.80 | baseline | — |
| Step 1, phase 1 parsing (timegm, stod) | 1 | 3 | 76.46 | 76.33–76.68 | about the same | — |
| Step 1, phase 1 parsing (timegm, stod) | 2 | 3 | 273.63 | 272.97–273.98 | 3.6× slower | 3.6× slower |
| Step 1, phase 1 parsing (timegm, stod) | 4 | 3 | 305.43 | 295.50–314.43 | 4.0× slower | 4.0× slower |
| Step 1, phase 1 parsing (timegm, stod) | 8 | 3 | 334.28 | 292.22–412.44 | 4.4× slower | 4.4× slower |
| Step 1, our parsing (objects) | 1 | 10 | 17.05 | 16.90–17.14 | 4.5× faster | — |
| Step 1, our parsing (objects) | 2 | 10 | 12.08 | 11.95–12.34 | 6.4× faster | 1.4× faster |
| Step 1, our parsing (objects) | 4 | 10 | 8.17 | 7.98–8.33 | 9.4× faster | 2.1× faster |
| Step 1, our parsing (objects) | 8 | 10 | 7.07 | 6.88–7.33 | 10.9× faster | 2.4× faster |
| Step 2, compact arrays | 1 | 10 | 15.51 | 15.28–15.58 | 5.0× faster | — |
| Step 2, compact arrays | 2 | 10 | 10.37 | 10.10–10.93 | 7.4× faster | 1.5× faster |
| Step 2, compact arrays | 4 | 10 | 6.28 | 6.00–7.04 | 12.2× faster | 2.5× faster |
| Step 2, compact arrays | 8 | 10 | 5.22 | 4.77–6.07 | 14.7× faster | 3.0× faster |

## 2. Search time: the distance report

The distance report = 5 searches, one per distance bucket (0–1, 1–3, 3–5, 5–10, over 10 miles); each scans all trips, counts the matches and adds up their fares, distances, tips and totals.

| Version | Threads | Distance report s (avg) | fastest–slowest | vs Phase 1 |
| --- | --- | --- | --- | --- |
| Phase 1 (objects, 1 thread) | 1 | 0.8071 | 0.55–1.83 | baseline |
| Step 1, phase 1 parsing (timegm, stod) | 1 | 1.6889 | 1.43–2.19 | 2.1× slower |
| Step 1, phase 1 parsing (timegm, stod) | 2 | 0.8807 | 0.59–1.26 | 1.1× slower |
| Step 1, phase 1 parsing (timegm, stod) | 4 | 1.5290 | 1.42–1.62 | 1.9× slower |
| Step 1, phase 1 parsing (timegm, stod) | 8 | 1.5576 | 0.70–2.86 | 1.9× slower |
| Step 1, our parsing (objects) | 1 | 1.8442 | 1.40–2.73 | 2.3× slower |
| Step 1, our parsing (objects) | 2 | 1.2584 | 0.70–2.42 | 1.6× slower |
| Step 1, our parsing (objects) | 4 | 0.8865 | 0.58–1.36 | 1.1× slower |
| Step 1, our parsing (objects) | 8 | 1.1520 | 0.80–1.87 | 1.4× slower |
| Step 2, compact arrays | 1 | 0.1467 | 0.15–0.15 | 5.5× faster |
| Step 2, compact arrays | 2 | 0.0845 | 0.08–0.10 | 9.5× faster |
| Step 2, compact arrays | 4 | 0.0462 | 0.04–0.05 | 17.5× faster |
| Step 2, compact arrays | 8 | 0.0325 | 0.03–0.04 | 24.8× faster |

## 2b. Search time: trip lookups by ID

1,000,000 random trip IDs (row numbers, the same in every run), all 18 fields read for each, repeated 5 times. First = round 1, right after the distance report; settled = round 5.

| Version | Threads | Runs | First s (avg) | Settled s (avg) | fastest–slowest settled | ns per lookup | vs Phase 1 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Phase 1 (objects, 1 thread) | 1 | 10 | 0.0707 | 0.0385 | 0.0369–0.0410 | 38 | baseline |
| Step 1, phase 1 parsing (timegm, stod) | 1 | 3 | 0.0404 | 0.0403 | 0.0395–0.0411 | 40 | about the same |
| Step 1, phase 1 parsing (timegm, stod) | 2 | 3 | 0.0247 | 0.0228 | 0.0218–0.0245 | 23 | 1.7× faster |
| Step 1, phase 1 parsing (timegm, stod) | 4 | 3 | 0.1362 | 0.1687 | 0.0112–0.4820 | 169 | 4.4× slower |
| Step 1, phase 1 parsing (timegm, stod) | 8 | 3 | 0.7049 | 0.1422 | 0.0104–0.4058 | 142 | 3.7× slower |
| Step 1, our parsing (objects) | 1 | 10 | 0.0405 | 0.0400 | 0.0383–0.0423 | 40 | about the same |
| Step 1, our parsing (objects) | 2 | 10 | 0.0242 | 0.0244 | 0.0220–0.0364 | 24 | 1.6× faster |
| Step 1, our parsing (objects) | 4 | 10 | 0.0353 | 0.0155 | 0.0109–0.0501 | 16 | 2.5× faster |
| Step 1, our parsing (objects) | 8 | 10 | 0.1185 | 0.0421 | 0.0099–0.1572 | 42 | 1.1× slower |
| Step 2, compact arrays | 1 | 10 | 0.0964 | 0.0821 | 0.0796–0.0871 | 82 | 2.1× slower |
| Step 2, compact arrays | 2 | 10 | 0.0918 | 0.0605 | 0.0531–0.0668 | 60 | 1.6× slower |
| Step 2, compact arrays | 4 | 10 | 0.0649 | 0.0525 | 0.0516–0.0561 | 52 | 1.4× slower |
| Step 2, compact arrays | 8 | 10 | 0.0684 | 0.0528 | 0.0525–0.0535 | 53 | 1.4× slower |

## 3. Memory (Signal 2)

Trip data = trips × bytes per trip (exact). Peak = whole program (macOS footprint).

| Version | Threads | Trip data MB | Bytes per trip | Peak MB | Peak ÷ data |
| --- | --- | --- | --- | --- | --- |
| Phase 1 (objects, 1 thread) | 1 | 2,109 | 216 | 3,562 | 1.69 |
| Step 1, phase 1 parsing (timegm, stod) | 1 | 2,109 | 216 | 4,222 | 2.00 |
| Step 1, phase 1 parsing (timegm, stod) | 2 | 2,109 | 216 | 4,222 | 2.00 |
| Step 1, phase 1 parsing (timegm, stod) | 4 | 2,109 | 216 | 4,222 | 2.00 |
| Step 1, phase 1 parsing (timegm, stod) | 8 | 2,109 | 216 | 3,513 | 1.67 |
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
| Load time | 48.1 s (47.46–48.40) |
| Trip data in memory | 3.80 GB |
| Peak memory | 5.62 GB |
| Same trips as Phase 1 objects (216 bytes each) | 14.4 GB: does not fit in this Mac's 8 GB |
| Distance report | 0.322 s |
| Trip lookups (1M, settled) | 17.271 s |

## 5. Did every version find the same trips?

Yes: all 92 phase 2 runs gave exactly the same trip count as phase 1 in each of the 5 distance buckets, and the same trip lookups (same average total of the looked-up trips).

## 6. Memory over time

| Recording | Run time s | Peak MB | Chart |
| --- | --- | --- | --- |
| phase1_objects | 81.5 | 5,570 | memory_results/memory_timeline.png |
| step1_objects_8threads | 6.5 | 4,230 | memory_results/memory_timeline.png |
| step2_compact_8threads | 5.7 | 931 | memory_results/memory_timeline.png |

## 7. Headline numbers

- **Signal 1, threads with phase 1 parsing:** loading took 76.5 s on 1 thread and 334.3 s on 8 threads (4.4× slower).
- **Signal 1, threads with our parsing:** 17.1 s on 1 thread, 7.1 s on 8 threads (2.4× faster); 10.9× faster than Phase 1 (76.8 s).
- **Signal 2, memory:** 2,109 MB of trip data as objects vs 557 MB compact (3.8× smaller).
- **Signal 2, search speed:** distance report 0.807 s in Phase 1 (objects) vs 0.147 s compact on 1 thread (5.5× faster) and 0.032 s at 8 threads (24.8× faster).
- **Trip lookups (all fields of a few trips):** objects 0.040 s vs compact 0.082 s for 1M lookups on 1 thread: compact is 2.1× slower here.
- **Signal 2, full file:** all 71,667,210 trips load in 3.8 GB compact; as objects they would need 14.4 GB.
