#include <cstdlib>
#include <iostream>
#include <parlay/parallel.h>
#include <parlay/sequence.h>
#include <chrono>
#include "primitives.h"
#include "quicksort.h"


int main() {
  // parlay::sequence<int> arr(3);
  // parlay::sequence<int> arr(10);
  // for (int i = 0; i < arr.size(); ++i) {
  //   arr[i] = i;
  // }
  // auto res = scan(arr, 0, arr.size());
  // print(res);
  // auto res2 = filter(arr, [](int x) { return x % 2 == 1; });
  // print(res2);
  // for (int i = 0; i < arr.size(); ++i) {
  //   arr[i] = arr.size() - i;
  //   arr[i] = rand() % 10000;
  // }
  // quicksort_par(arr);
  // quicksort(arr);
  // print(arr);
  parlay::sequence<int> arr(1e8);
  for (int i = 0; i < arr.size(); ++i)
    arr[i] = rand() % 10000;

  constexpr int N = 5;
  float d1t = 0, d2t = 0;
  // parlay::execute_with_scheduler(2, [&] {
    for (int i = 0; i < N; ++i) {
      auto arr2 = arr;
      auto start2 = std::chrono::high_resolution_clock::now();
      quicksort_par(arr2);
      // quicksort(arr2);
      auto end2 = std::chrono::high_resolution_clock::now();
      auto d2 = end2 - start2;
      d2t += d2.count();
    }
  // });

  for (int i = 0; i < N; ++i) {
    auto arr1 = arr;
    auto start1 = std::chrono::high_resolution_clock::now();
    quicksort(arr1);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto d1 = end1 - start1;
    d1t += d1.count();
  }

  // for (int i = 0; i < arr.size(); ++i) {
  //   if (arr1[i] != arr2[i]) {
  //     std::cout << "diff at " << i << "\n";
  //     return 1;
  //   }
  // }
  std::cout << d1t << ' ' << d2t << "\n";
  std::cout << d1t / d2t << "\n";
}
