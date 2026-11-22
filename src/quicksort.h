#pragma once
#include "parlay/parallel.h"

enum class Algorithm { Parallel, Serial };

template <Algorithm kind, typename It> 
void quicksort(It l, It r) {
  auto diff = --r - l;
  if (diff <= 0) return;
  auto pivot = *(l + diff / 2);
  It i = l;
  It j = r;
  while (i <= j) {
    while (*i < pivot) i++;
    while (*j > pivot) j--;
    if (i >= j) break;
    std::swap(*(i++), *(j--));
  }
  auto left = [&]() { quicksort<kind>(l, j + 1); };
  auto right = [&]() { quicksort<kind>(j + 1, r + 1); };
  if constexpr (kind == Algorithm::Parallel)
    if (r - l >= 64) return parlay::par_do(left, right);
  left(), right();
}
