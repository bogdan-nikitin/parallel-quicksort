#include "parlay/sequence.h"
#include "quicksort.h"
#include <algorithm>
#include <cstdlib>
#include <gtest/gtest.h>

static parlay::sequence<int> random_array(size_t n, int max) {
  parlay::sequence<int> arr(n);
  for (size_t i = 0; i < n; ++i)
    arr[i] = rand() % max;
  return arr;
}

template<typename F>
static float measure_time(F f) {
  auto start = std::chrono::high_resolution_clock::now();
  f();
  auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

TEST(CorrectnessSerial, BasicTest) {
  parlay::sequence<int> arr = {4, 7, 1, 5, 3, 2, 2, -1, 8};
  quicksort<Algorithm::Serial>(arr.begin(), arr.end());
  parlay::sequence<int> expected = {-1, 1, 2, 2, 3, 4, 5, 7, 8};
  EXPECT_EQ(arr, expected);
}

TEST(CorrectnessSerial, ReverseTest) {
  parlay::sequence<int> arr(10000);
  parlay::sequence<int> expected(10000);
  for (size_t i = 0; i < arr.size(); ++i) {
    arr[i] = arr.size() - 1 - i;
    expected[i] = i;
  }
  quicksort<Algorithm::Serial>(arr.begin(), arr.end());
  EXPECT_EQ(arr, expected);
}

TEST(CorrectnessSerial, RandomTest) {
  for (size_t i = 10; i <= 10000; i *= 10) {
    parlay::sequence<int> arr = random_array(i, 1000000);
    parlay::sequence<int> expected = arr;
    std::sort(expected.begin(), expected.end());
    quicksort<Algorithm::Serial>(arr.begin(), arr.end());
    EXPECT_EQ(arr, expected);
  }
}

TEST(CorrectnessParallel, RandomTest) {
  for (size_t i = 10; i <= 10000; i *= 10) {
    parlay::sequence<int> arr = random_array(i, 1000000);
    parlay::sequence<int> expected = arr;
    std::sort(expected.begin(), expected.end());
    quicksort<Algorithm::Parallel>(arr.begin(), arr.end());
    EXPECT_EQ(arr, expected);
  }
}

TEST(Performance, PerformanceTest) {
  parlay::sequence<int> arr;
  parlay::sequence<int> expected;
  parlay::sequence<int> copy;
  constexpr int N = 5;
  float t_ser = 0, t_par = 0;

  parlay::execute_with_scheduler(4, [&] {
    for (int i = 0; i < N; ++i) {
      arr = random_array(1e8, 1e8);
      expected = arr;
      std::sort(expected.begin(), expected.end());

      copy = arr;
      t_par += measure_time([&]() {
        quicksort<Algorithm::Parallel>(copy.begin(), copy.end());
      });
      EXPECT_EQ(copy, expected);

      copy = arr;
      t_ser += measure_time([&]() {
        quicksort<Algorithm::Serial>(copy.begin(), copy.end());
      });
      EXPECT_EQ(copy, expected);
    }
  });

  auto ratio = t_ser / t_par;

  std::cout << "Average parallel: " << t_par / N << " ms\n";
  std::cout << "Average serial:   " << t_ser / N << " ms\n";
  std::cout << "Ratio: " << ratio << "\n";
  EXPECT_GE(ratio, 3);
}
