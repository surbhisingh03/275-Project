# Results summary

Test file: 10,238,173 trips. Times are averages over all runs (fastest–slowest in brackets where useful).

## 1. Load time (Signal 1)

| Version | Threads | Runs | Load s (avg) | fastest–slowest | vs Phase 1 | vs same version, 1 thread |
| --- | --- | --- | --- | --- | --- | --- |
| Phase 1 (objects, 1 thread) | 1 | 10 | 76.31 | 75.73–77.26 | baseline | — |
| Step 1, phase 1 parsing (timegm, stod) | 1 | 3 | 76.17 | 75.98–76.28 | about the same | — |
| Step 1, phase 1 parsing (timegm, stod) | 2 | 3 | 271.97 | 265.77–277.87 | 3.6× slower | 3.6× slower |
| Step 1, phase 1 parsing (timegm, stod) | 4 | 3 | 302.62 | 294.50–317.28 | 4.0× slower | 4.0× slower |
| Step 1, phase 1 parsing (timegm, stod) | 8 | 3 | 276.76 | 263.70–293.16 | 3.6× slower | 3.6× slower |
| Step 1, our parsing (objects) | 1 | 10 | 16.78 | 16.70–16.90 | 4.5× faster | — |
| Step 1, our parsing (objects) | 2 | 10 | 11.58 | 11.49–11.68 | 6.6× faster | 1.4× faster |
| Step 1, our parsing (objects) | 4 | 10 | 7.84 | 7.45–8.34 | 9.7× faster | 2.1× faster |
| Step 1, our parsing (objects) | 8 | 10 | 7.44 | 7.18–7.64 | 10.3× faster | 2.3× faster |
| Step 2, compact arrays | 1 | 10 | 15.22 | 15.18–15.29 | 5.0× faster | — |
| Step 2, compact arrays | 2 | 10 | 9.89 | 9.36–10.58 | 7.7× faster | 1.5× faster |
| Step 2, compact arrays | 4 | 10 | 5.98 | 5.80–6.86 | 12.8× faster | 2.5× faster |
| Step 2, compact arrays | 8 | 10 | 5.64 | 5.42–6.71 | 13.5× faster | 2.7× faster |

## 2. Search times

Distance report = the 5 distance buckets (5 searches). Time windows start 10 March 2014 (1 month = all of March).

Cache off = the same program run with MallocLargeCache=0 (see section 3); only step 1 and step 2 were run that way.

| Version | Threads | Distance report s | 1 hour s | 1 day s | 1 week s | 1 month s | Report vs Phase 1 | Report s, cache off | 1 month s, cache off |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Phase 1 (objects, 1 thread) | 1 | 0.5273 | 0.0500 | 0.0503 | 0.0500 | 0.0478 | baseline | — | — |
| Step 1, phase 1 parsing (timegm, stod) | 1 | 3.3088 | 0.6555 | 0.6688 | 0.6451 | 0.6784 | 6.3× slower | — | — |
| Step 1, phase 1 parsing (timegm, stod) | 2 | 2.1834 | 0.3941 | 0.3916 | 0.3940 | 0.3934 | 4.1× slower | — | — |
| Step 1, phase 1 parsing (timegm, stod) | 4 | 1.2701 | 0.0909 | 0.0369 | 0.0363 | 0.0382 | 2.4× slower | — | — |
| Step 1, phase 1 parsing (timegm, stod) | 8 | 1.0701 | 0.0754 | 0.0662 | 0.0923 | 0.0769 | 2.0× slower | — | — |
| Step 1, our parsing (objects) | 1 | 2.8855 | 0.4743 | 0.4779 | 0.4713 | 0.4799 | 5.5× slower | 0.5918 | 0.0496 |
| Step 1, our parsing (objects) | 2 | 1.9177 | 0.1403 | 0.1033 | 0.0942 | 0.0730 | 3.6× slower | 0.3612 | 0.0403 |
| Step 1, our parsing (objects) | 4 | 1.1250 | 0.0831 | 0.0724 | 0.0399 | 0.0402 | 2.1× slower | 0.2736 | 0.0363 |
| Step 1, our parsing (objects) | 8 | 0.9265 | 0.0993 | 0.0651 | 0.0517 | 0.0511 | 1.8× slower | 0.1992 | 0.0308 |
| Step 2, compact arrays | 1 | 0.1455 | 0.0063 | 0.0046 | 0.0051 | 0.0069 | 3.6× faster | 0.1451 | 0.0070 |
| Step 2, compact arrays | 2 | 0.0836 | 0.0034 | 0.0024 | 0.0029 | 0.0038 | 6.3× faster | 0.0778 | 0.0038 |
| Step 2, compact arrays | 4 | 0.0441 | 0.0015 | 0.0014 | 0.0019 | 0.0029 | 12.0× faster | 0.0421 | 0.0028 |
| Step 2, compact arrays | 8 | 0.0455 | 0.0018 | 0.0015 | 0.0018 | 0.0030 | 11.6× faster | 0.0295 | 0.0026 |

## 3. Memory (Signal 2 and Signal 3)

Trip data = trips × bytes per trip (exact). Peak = whole program (macOS footprint). Cache off = run with MallocLargeCache=0 (macOS does not keep freed big blocks).

| Version | Threads | Trip data MB | Bytes per trip | Peak MB | Peak ÷ data | Peak MB, cache off | Peak ÷ data, cache off |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Phase 1 (objects, 1 thread) | 1 | 2,109 | 216 | 4,965 | 2.35 | — | — |
| Step 1, phase 1 parsing (timegm, stod) | 1 | 2,109 | 216 | 4,222 | 2.00 | — | — |
| Step 1, phase 1 parsing (timegm, stod) | 2 | 2,109 | 216 | 4,222 | 2.00 | — | — |
| Step 1, phase 1 parsing (timegm, stod) | 4 | 2,109 | 216 | 4,222 | 2.00 | — | — |
| Step 1, phase 1 parsing (timegm, stod) | 8 | 2,109 | 216 | 4,222 | 2.00 | — | — |
| Step 1, our parsing (objects) | 1 | 2,109 | 216 | 4,222 | 2.00 | 4222 | 2.00 |
| Step 1, our parsing (objects) | 2 | 2,109 | 216 | 4,222 | 2.00 | 3167 | 1.50 |
| Step 1, our parsing (objects) | 4 | 2,109 | 216 | 4,222 | 2.00 | 2640 | 1.25 |
| Step 1, our parsing (objects) | 8 | 2,109 | 216 | 4,223 | 2.00 | 2377 | 1.13 |
| Step 2, compact arrays | 1 | 557 | 57 | 608 | 1.09 | 598 | 1.07 |
| Step 2, compact arrays | 2 | 557 | 57 | 1,117 | 2.01 | 579 | 1.04 |
| Step 2, compact arrays | 4 | 557 | 57 | 1,049 | 1.89 | 570 | 1.02 |
| Step 2, compact arrays | 8 | 557 | 57 | 929 | 1.67 | 568 | 1.02 |

## 4. Step 2 on the full file (Signal 2)

| Measure | Value |
| --- | --- |
| Trips | 71,667,210 |
| Runs (8 threads) | 3 |
| Load time | 44.7 s (44.13–45.84) |
| Trip data in memory | 3.80 GB |
| Peak memory | 5.78 GB |
| Peak memory, cache off | 3.85 GB |
| Same trips as Phase 1 objects (216 bytes each) | 14.4 GB: does not fit in this Mac's 8 GB |
| Distance report | 0.268 s |
| 1-month time search | 0.0184 s |

## 5. Did every version find the same trips?

Yes: all 92 phase 2 runs gave exactly the same counts as phase 1 (9 searches: 5 distance buckets + 4 time windows).

## 6. Memory over time

| Recording | Run time s | Peak MB | Chart |
| --- | --- | --- | --- |
| phase1_objects | 76.4 | 3,841 | memory_results/memory_timeline.png |
| step1_objects_8threads | 5.8 | 4,223 | memory_results/memory_timeline.png |
| step2_compact_8threads | 4.5 | 924 | memory_results/memory_timeline.png |

## 7. Headline numbers

- **Signal 1, threads with phase 1 parsing:** loading took 76.2 s on 1 thread and 276.8 s on 8 threads (3.6× slower).
- **Signal 1, threads with our parsing:** 16.8 s on 1 thread, 7.4 s on 8 threads (2.3× faster); 10.3× faster than Phase 1 (76.3 s).
- **Signal 2, memory:** 2,109 MB of trip data as objects vs 557 MB compact (3.8× smaller).
- **Signal 2, search speed:** distance report 0.527 s in Phase 1 (objects) vs 0.145 s compact on 1 thread (3.6× faster) and 0.044 s at 4 threads (12.0× faster).
- **Signal 2, full file:** all 71,667,210 trips load in 3.8 GB compact; as objects they would need 14.4 GB.
- **Signal 3, macOS keeps freed memory:** step 1 (8 threads) peaks at 2.00× its trip data normally vs 1.13× with MallocLargeCache=0; its distance report (1 thread) takes 2.89 s normally vs 0.59 s with the cache off.
- **Signal 3, full file:** step 2 peaks at 5.78 GB normally vs 3.85 GB with the cache off (trip data 3.80 GB).
