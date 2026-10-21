# Parallel Systems & High-Performance Computing

C++ labs and a final project on **performance engineering** at every level of parallelism: **SIMD vectorisation** (SSE/AVX intrinsics), **shared-memory parallelism** (OpenMP), **cache-aware data layout** and **distributed memory** with **MPI**.

## Final project — Parallel Conway's Game of Life

[`project/`](project) · C++20 · CMake · OpenMP · ncurses

- Finite grid with periodic (toroidal) borders; sequential and **multithreaded** engines behind a common interface.
- Optional **cache-line padding** to avoid false sharing between threads.
- Terminal UI (ncurses) demo mode and a benchmark mode that compares all four variants.

```
=== BENCHMARK: GRID 500x500 (50 iters) ===
Sequential without padding: 0.97 s
Sequential with padding:    0.90 s
Multithreaded without pad:  0.61 s
Multithreaded with padding: 0.59 s     (4 threads)
```

## Labs

| Lab | Task | Topic |
|---|---|---|
| **1 — SIMD** | `task_1` | Cosine similarity of large vectors with **SSE/AVX intrinsics** (auto-vectorisation disabled to measure the manual version) |
| | `task_2` | Minimum enclosing ball and farthest point of a d-dimensional point cloud, vectorised with AVX |
| **2 — OpenMP** | `task_1` | Riemann-sum approximation of π: sequential vs OpenMP vs manually unrolled/vectorised + OpenMP |
| | `task_2` | Parallel **counting sort**; avoiding **false sharing** with cache-line-aligned counters (2.3× vs naïve) |
| | `task_3` | Parallel binary-tree height / balance check with **OpenMP tasks** and a cut-off threshold |
| **3 — MPI** | `task_1` | Custom **barrier** implemented with point-to-point messages |
| | `task_234` | Stream processing with MPI: **pipeline** of modules and **farm** (emitter–workers–collector) to remove the bottleneck, with service-time analysis |

## Building

```bash
# Labs: each task has its own makefile (g++ / mpicxx)
cd lab2/task_2 && make            # compiles and runs with default parameters

# Final project
cd project
cmake -S . -B build && cmake --build build
./build/GameOfLife --demo 50 50
./build/GameOfLife --benchmark 2000 2000 500 8   # width height iterations threads
```

Requirements: g++ with OpenMP and AVX support, CMake ≥ 3.10, ncurses (project), and an MPI implementation (e.g. OpenMPI) for lab 3.

## Tech

C++20 · SSE/AVX intrinsics · OpenMP · MPI · ncurses · CMake · Make

*Coursework from Parallel Systems at Mälardalen University, Sweden.*
