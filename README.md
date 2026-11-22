# Parallel Quicksort

Build using CMake:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run:
```bash
./build/test/quicksort_test
```

Measurements on AMD Ryzen 7 9800X3D 8-Core Processor, 
4700 Mhz, 8 Core(s), 16 Logical Processor(s):
```
Average parallel: 1947.2 ms
Average serial:   6974.2 ms
Ratio: 3.58166
```
