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
2090           7696         3.6823
2229           7717         3.46209
2215           7776         3.51061
2087           7823         3.74844
2280           7760         3.40351

Average parallel: 2180.2 ms
Average serial:   7754.4 ms
Ratio: 3.55674
```
