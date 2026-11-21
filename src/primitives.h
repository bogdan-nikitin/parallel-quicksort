#pragma once
#include <algorithm>
#include <cassert>
#include <parlay/parallel.h>
#include <parlay/sequence.h>
#include <utility>

// constexpr int BLOCK = 3;
constexpr int BLOCK = 10000;

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

template <typename T, typename U, typename F>
void map_impl(const parlay::sequence<T> &in, std::vector<U> &out, int l,
              int r, F &&f) {
  if (r - l <= 1000) {
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
  if (r - l <= 1) {
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

template <typename F> void blocked_for(int l, int r, int block, F &&f) {
  int blocks = (r - l + block - 1) / block;
  parallel_for(0, blocks, [&](int i) {
    int s = l + i * block;
    int e = std::min(s + block, r);
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
                         parlay::sequence<T> &out, int l, int r, T init = {}) {
  T next = in[l] + init;
  out[l] = {};
  for (int i = l; i < r - 1; ++i) {
    T cur = next;
    next = in[i + 1];
    out[i + 1] = out[i] + cur;
  }
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
  constexpr int block = 1000;
  parlay::sequence<T> sums((r - l + block - 1) / block);
  blocked_for(l, r, block,
              [&](int i, int s, int e) { sums[i] = reduce_serial(a, s, e); });
  sums = scan(sums, 0, sums.size());
  // print(sums);
  parlay::sequence<T> res(r - l);
  blocked_for(l, r, block, [&](int i, int s, int e) {
    // printf("scan %i %i\n", s, e);
    res[s] = sums[i];
    scan_serial_inplace(a, res, s, e);
  });
  return res;
}

template <typename T>
void scan_inplace(parlay::sequence<T> &a, int l, int r) {
  if (r - l <= BLOCK) {
    scan_serial_inplace(a, a, l, r);
    return;
  }
  constexpr int block = 1000;
  parlay::sequence<T> sums((r - l + block - 1) / block);
  blocked_for(l, r, block,
              [&](int i, int s, int e) { sums[i] = reduce_serial(a, s, e); });
  sums = scan(sums, 0, sums.size());
  // print(sums);
  blocked_for(l, r, block, [&](int i, int s, int e) {
    // printf("scan %i %i\n", s, e);
    // a[s] = sums[i];
    scan_serial_inplace(a, a, s, e, sums[i]);
  });
}

template <typename T, typename P>
parlay::sequence<T> filter(const parlay::sequence<T> &in, P &&pred) {
  // /*thread_local */parlay::sequence<int> flags;
  // flags.resize(in.size());
  // map_impl(in, flags, 0, in.size(), pred);
  auto flags = map<bool>(in, std::forward<P>(pred));
  // parlay::sequence<int> sums(blocks_count(in.size()));
  constexpr int block = 1000;
  parlay::sequence<int> sums((in.size() + block - 1) / block);
  auto p = flags.data();
  blocked_for(0, in.size(), block, [&](int i, int s, int e) {
    // printf("B %i %i %i\n", in.size(), s, e);
    int count = 0;
    for (int j = s; j < e; ++j)
      count += flags[j];
    sums[i] = count;
  });
  auto pref = scan(sums, 0, sums.size());
  parlay::sequence<T> res(pref.back() + sums.back());
  // T last = sums.back();
  // scan_inplace(sums, 0, sums.size());
  // parlay::sequence<T> res(last + sums.back());
  blocked_for(0, in.size(), block, [&](int i, int s, int e) {
    int pos = pref[i];
    // int pos = sums[i];
    for (int j = s; j < e; ++j) {
      if (p[j])
        res[pos++] = in[j];
    }
  });
  return res;
}
