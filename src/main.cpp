#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <parlay/parallel.h>
#include <parlay/sequence.h>
#include <chrono>
#include "quicksort.h"


int main() {
  parlay::sequence<int> arr(1e8);
  for (int i = 0; i < arr.size(); ++i)
    arr[i] = rand() % 10000;
  parlay::sequence<int> target = arr;
  std::sort(target.begin(), target.end());

  constexpr int N = 5;
  float d1t = 0, d2t = 0;
  parlay::execute_with_scheduler(4, [&] {
    for (int i = 0; i < N; ++i) {
      auto arr2 = arr;
      auto start2 = std::chrono::high_resolution_clock::now();
      quicksort<Algorithm::Parallel>(arr2.begin(), arr2.end());
      auto end2 = std::chrono::high_resolution_clock::now();
      auto d2 = end2 - start2;
      d2t += d2.count();
      for (int i = 0; i < arr.size(); ++i) {
        if (target[i] != arr2[i]) {
          std::cout << "diff at " << i << "\n";
          std::abort();
        }
      }
    }
  });

  for (int i = 0; i < N; ++i) {
    auto arr1 = arr;
    auto start1 = std::chrono::high_resolution_clock::now();
    quicksort<Algorithm::Serial>(arr1.begin(), arr1.end());
    auto end1 = std::chrono::high_resolution_clock::now();
    auto d1 = end1 - start1;
    d1t += d1.count();
  }
  std::cout << d1t << ' ' << d2t << "\n";
  std::cout << d1t / d2t << "\n";
}
