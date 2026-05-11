# Conway's Game of Life (C++ Parallel Implementation)

An implementation of Conway's Game of Life in C++. 

It uses a finite grid with periodic borders (that wrap around).

It supports multithreading and padding for better cache alignment.

## To build:
```bash
rm -rf build
cmake -S . -B build
cmake --build build
```

## To use:
```
./build/GameOfLife --demo 50 50
./build/GameOfLife --benchmark 2000 2000 500 8
```