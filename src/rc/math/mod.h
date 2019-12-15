#pragma once

#include "rc/core.h"

namespace rc::math {

constexpr static usize _[] = {0, num::max_value<usize>()};

template <usize N>
struct NDDims {
  constexpr static usize RANK = N;
  usize _0[RANK];

  auto operator[](usize idx) const -> usize {
    if (idx >= RANK) return 1u;
    return _0[idx];
  }

  auto count() const -> usize { return ops::prod(_0); }

  auto operator==(const NDDims& other) -> bool {
    return cmp::all_eq(_0, other._0, N);
  }

  auto operator!=(const NDDims& other) -> bool {
    return cmp::any_ne(_0, other._0, N);
  }
  template <class Out>
  void fmt(fmt::Formatter<Out>& formatter) const {
    formatter.write_val(_0);
  }
};

template <usize N>
struct NDStep {
  constexpr static usize RANK = N;

  usize _0[RANK];

  static auto from_dims(const NDDims<RANK>& size) -> NDStep {
    NDStep res = {{1u}};
    if constexpr (RANK > 1) {
      for (usize i = 1; i < RANK; ++i) {
        res._0[i] = res._0[i - 1] * size._0[i - 1];
      }
    }
    return res;
  }

  auto operator[](usize idx) const -> usize {
    if (idx >= RANK) return 0u;
    return _0[idx];
  }

  auto operator==(const NDStep& other) -> bool {
    return cmp::all_eq(_0, other._0, N);
  }

  auto operator!=(const NDStep& other) -> bool {
    return cmp::any_ne(_0, other._0, N);
  }

  template <class Out>
  void fmt(fmt::Formatter<Out>& formatter) const {
    formatter.write_val(_0);
  }
};

template <usize N>
struct NDIdxs {
  constexpr static usize RANK = N;
  usize _0[RANK];

  auto operator^(const NDStep<RANK>& step) const -> usize {
    static_assert(RANK <= 10);

    auto ret = _0[0] * step._0[0];
    if constexpr (RANK > 1) ret += _0[1] * step._0[1];
    if constexpr (RANK > 2) ret += _0[2] * step._0[2];
    if constexpr (RANK > 3) ret += _0[3] * step._0[3];
    if constexpr (RANK > 4) ret += _0[4] * step._0[4];
    if constexpr (RANK > 5) ret += _0[5] * step._0[5];
    if constexpr (RANK > 6) ret += _0[6] * step._0[6];
    if constexpr (RANK > 7) ret += _0[7] * step._0[7];
    if constexpr (RANK > 8) ret += _0[8] * step._0[8];
    if constexpr (RANK > 9) ret += _0[9] * step._0[9];
    return ret;
  }

  template <class Out>
  void fmt(fmt::Formatter<Out>& formatter) const {
    formatter.write_val(_0);
  }
};

#pragma region Range
struct Range {
  usize _start;
  usize _end;

  template <usize N>
  static auto from(const usize (&s)[N]) -> Range {
    static_assert(N == 1 || N == 2);
    if constexpr (N == 1) {
      return {s[0], s[0] + 1};
    }
    if constexpr (N == 2) {
      rc::assert(s[0] <= s[1], u8"rc::math::Range: invalid range");
      return {s[0], s[1]};
    }
  }

  auto wrap(usize size) const -> Range {
    rc::assert(_start < size, u8"rc::math::Range: out of range");
    return Range{_start, cmp::min(_end, size)};
  }

  auto idx() const noexcept -> usize { return _start; }
  auto len() const noexcept -> usize { return _end - _start; }
};

#pragma endregion

}  // namespace rc::math
