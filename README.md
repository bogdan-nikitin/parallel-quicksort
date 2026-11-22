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

Measurements (5 runs average, 4 threads) on AMD Ryzen 7 9800X3D 8-Core Processor, 
4700 Mhz, 8 Core(s), 16 Logical Processors:
```
Parallel, ms   Serial, ms   Ratio
1969           7207         3.66023
2000           6839         3.4195
1996           6908         3.46092
1880           6882         3.66064
1963           6893         3.51146

Average parallel: 1961.6 ms
Average serial:   6945.8 ms
Ratio: 3.54088
```
