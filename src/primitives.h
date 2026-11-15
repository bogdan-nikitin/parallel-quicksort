#pragma once
#include <algorithm>
#include <cassert>
#include <parlay/parallel.h>
#include <parlay/sequence.h>
#include <utility>

// constexpr int BLOCK = 3;
constexpr int BLOCK = 1000;

namespace {
template <typename T, typename U, typename F>
void map_impl(const parlay::sequence<T> &in, parlay::sequence<U> &out, int l,
              int r, F &&f) {
  if (r - l <= BLOCK) {
    for (int i = l; i < r; ++i)
      out[i] = std::forward<F>(f)(in[i]);
  } else {
    int m = (l + r) / 2;
    parlay::par_do([&] { map_impl(in, out, l, m, std::forward<F>(f)); },
                   [&] { map_impl(in, out, m, r, std::forward<F>(f)); });
  }
}

int blocks_count(int n) { return (n + BLOCK - 1) / BLOCK; }

} // namespace

template <typename U, typename T, typename F>
parlay::sequence<U> map(const parlay::sequence<T> &in, F &&f) {
  parlay::sequence<U> out(in.size());
  map_impl(in, out, 0, in.size(), std::forward<F>(f));
  return out;
}

template <typename F> void parallel_for(int l, int r, F &&f) {
  // if (l + 1 == r) {
  //   std::forward<F>(f)(l);
  // } else if (l < r) {
  //   int m = (l + r) / 2;
  //   parlay::par_do([&] { parallel_for(l, m, std::forward<F>(f)); },
  //                  [&] { parallel_for(m, r, std::forward<F>(f)); });
  // }
  if (r - l <= BLOCK) {
    for (int i = l; i < r; ++i)
      f(i);
  } else {
    int m = (l + r) / 2;
    parlay::par_do([&] { parallel_for(l, m, std::forward<F>(f)); },
                   [&] { parallel_for(m, r, std::forward<F>(f)); });
  }
}

template <typename F> void blocked_for(int l, int r, F &&f) {
  int blocks = blocks_count(r - l);
  parallel_for(0, blocks, [&](int i) {
    int s = l + i * BLOCK;
    int e = std::min(s + BLOCK, r);
    std::forward<F>(f)(i, s, e);
  });
}

template <typename T>
T reduce_serial(const parlay::sequence<T> &a, int l, int r) {
  T s{};
  for (int i = l; i < r; ++i)
    s += a[i];
  return s;
}

template <typename T>
void scan_serial_inplace(const parlay::sequence<T> &in,
                         parlay::sequence<T> &out, int l, int r) {
  for (int i = l; i < r - 1; ++i)
    out[i + 1] = out[i] + in[i];
}

// exclusive
template <typename T>
parlay::sequence<T> scan_serial(const parlay::sequence<T> &in, int l, int r) {
  parlay::sequence<T> out(r - l);
  for (int i = 0; i < r - l - 1; ++i)
    out[i + 1] = out[i] + in[l + i];
  return out;
}

template <typename T> void print(const parlay::sequence<T> &a) {
  for (int i = 0; i < a.size(); ++i)
    std::cout << a[i] << ' ';
  std::cout << '\n';
}

template <typename T>
parlay::sequence<T> scan(const parlay::sequence<T> &a, int l, int r) {
  if (r - l <= BLOCK)
    return scan_serial(a, l, r);
  parlay::sequence<T> sums(blocks_count(r - l));
  blocked_for(l, r,
              [&](int i, int s, int e) { sums[i] = reduce_serial(a, s, e); });
  sums = scan(sums, 0, sums.size());
  // print(sums);
  parlay::sequence<T> res(r - l);
  blocked_for(l, r, [&](int i, int s, int e) {
    // printf("scan %i %i\n", s, e);
    scan_serial_inplace(a, res, s, e);
    for (int j = s; j < e; ++j)
      res[j] += sums[i];
  });
  return res;
}

template <typename T, typename P>
parlay::sequence<T> filter(const parlay::sequence<T> &in, P &&pred) {
  auto flags = map<bool>(in, std::forward<P>(pred));
  parlay::sequence<int> sums(blocks_count(in.size()));
  blocked_for(0, in.size(), [&](int i, int s, int e) {
    int count = 0;
    for (int j = s; j < e; ++j)
      count += flags[j];
    sums[i] = count;
  });
  auto pref = scan(sums, 0, sums.size());
  parlay::sequence<T> res(pref.back() + sums.back());
  blocked_for(0, in.size(), [&](int i, int s, int e) {
    int pos = pref[i];
    for (int j = s; j < e; ++j) {
      if (flags[j])
        res[pos++] = in[j];
    }
  });
  return res;
}
