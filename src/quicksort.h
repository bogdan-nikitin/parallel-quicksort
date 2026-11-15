#pragma once
#include "parlay/parallel.h"
#include "primitives.h"
#include <functional>

template <typename T> void quicksort(parlay::sequence<T> &a, int l, int r) {
  if (l >= r)
    return;
  T pivot = a[(l + r) / 2];
  int i = l;
  int j = r;
  while (i <= j) {
    while (a[i] < pivot)
      i++;
    while (a[j] > pivot)
      j--;
    if (i >= j)
      break;
    std::swap(a[i++], a[j--]);
  }
  quicksort(a, l, j);
  quicksort(a, j + 1, r);
}

template <typename T> void quicksort(parlay::sequence<T> &a) {
  quicksort(a, 0, a.size() - 1);
}

template <typename T> void quicksort_par(parlay::sequence<T> &a) {
  if (a.size() <= 2) {
    // std::cout<<"{\n";
    // print(a);
    quicksort(a);
    // print(a);
    // std::cout<<"}\n";
    return;
  }
  T pivot = a[a.size() / 2];
  // parlay::sequence<T> points(101);
  // int step = a.size() / 101;
  // parallel_for(0, 101, [&](int i) {
  //   points[i] = a[i * step];
  // });
  // quicksort(points);
  // T pivot = points[50];

  // parlay::sequence<T> left, middle, right;
  // parlay::par_do3(
  //     [&]() { left = filter(a, [&](const T &x) { return x < pivot; }); },
  //     [&]() { middle = filter(a, [&](const T &x) { return x == pivot; }); },
  //     [&]() { right = filter(a, [&](const T &x) { return x > pivot; }); });
  auto left = filter(a, [&](const T& x) { return x < pivot; });
  auto middle = filter(a, [&](const T& x) { return x == pivot; });
  auto right = filter(a, [&](const T& x) { return x > pivot; });
  // std::cout << "SORT " << pivot << "\n";
  // print(left);
  // print(middle);
  // print(right);
  parlay::par_do([&]() { quicksort_par(left); },
                 [&]() { quicksort_par(right); });
  blocked_for(0, a.size(), [&](int, int s, int r) {
    for (int j = s; j < r; ++j) {
      if (j < left.size())
        a[j] = left[j];
      else if (j - left.size() < middle.size())
        a[j] = middle[j - left.size()];
      else
        a[j] = right[j - left.size() - middle.size()];
    }
  });
}
