#pragma once

#include "sfc/core/chr.h"
#include "sfc/core/slice.h"

namespace sfc::str {

using slice::Range;
using chr::Chars;

template <class T>
struct FromStr {
  static auto from_str(Str) -> Option<T>;
};

struct Str {
  const char* _ptr = nullptr;
  usize _len = 0;

 public:
  constexpr Str() noexcept = default;

  constexpr Str(const char* s, usize n) noexcept : _ptr{s}, _len{n} {}

  constexpr Str(const char* s) noexcept : _ptr{s}, _len{s == nullptr ? 0 : __builtin_strlen(s)} {}

  constexpr static auto from_utf8(Slice<const u8> s) noexcept -> Str {
    return Str{(const char*)s._ptr, s._len};
  }

  constexpr static auto from_cstr(const char* s) noexcept -> Str {
    const auto n = s ? __builtin_strlen(s) : 0;
    return Str{s, n};
  }

 public:
  constexpr auto ptr() const noexcept -> const char* {
    return _ptr;
  }

  constexpr auto len() const noexcept -> usize {
    return _len;
  }

  constexpr auto as_ptr() const noexcept -> const char* {
    return _ptr;
  }

  constexpr auto as_str() const noexcept -> Str {
    return *this;
  }

  constexpr auto data() const noexcept -> const char* {
    return _ptr;
  }

  constexpr auto size() const noexcept -> usize {
    return _len;
  }

  constexpr auto is_empty() const noexcept -> bool {
    return _len == 0;
  }

  constexpr auto as_bytes() const noexcept -> Slice<const u8> {
    return {ptr::cast<const u8>(_ptr), _len};
  }

 public:
  constexpr auto operator[](usize idx) const noexcept -> char {
    if (!_ptr || idx >= _len) return 0;
    return _ptr[idx];
  }

  constexpr auto operator[](Range ids) const noexcept -> Str {
    ids = ids.wrap(_len);
    return Str{_ptr + ids._start, ids.len()};
  }

  auto split_at(usize mid) const noexcept -> Tuple<Str, Str> {
    const auto x = mid < _len ? mid : _len;
    return Tuple{Str{_ptr, x}, Str{_ptr + x, _len - x}};
  }

 public:
  constexpr auto operator==(Str s) const noexcept -> bool {
    if (_len != s._len) return false;
    if (_len == 0 || _ptr == s._ptr) return true;
    return __builtin_memcmp(_ptr, s._ptr, _len) == 0;
  }

  constexpr auto operator<=>(Str other) const noexcept -> int {
    const auto min_len = _len < other._len ? _len : other._len;
    if (min_len != 0) {
      const auto ret = __builtin_memcmp(_ptr, other._ptr, min_len);
      if (ret != 0) {
        return ret < 0 ? -1 : 1;
      }
    }
    return _len == other._len ? 0 : (_len < other._len ? -1 : 1);
  }

  using Iter = slice::Iter<const char>;
  auto iter() const noexcept -> slice::Iter<const char> {
    return Iter{_ptr, _len};
  }

  auto chars() const noexcept -> Chars {
    return Chars{{}, this->as_bytes()};
  }

 public:
  auto find(auto&& pat) const -> Option<usize>;
  auto rfind(auto&& pat) const -> Option<usize>;

  auto contains(auto&& pat) const -> bool;
  auto starts_with(auto&& pat) const -> bool;
  auto ends_with(auto&& pat) const -> bool;

  auto trim_start() const noexcept -> Str;
  auto trim_start_matches(auto&& pat) const -> Str;

  auto trim_end() const noexcept -> Str;
  auto trim_end_matches(auto&& pat) const -> Str;

  auto trim() const noexcept -> Str;
  auto trim_matches(auto&& pat) const -> Str;

  auto split_once(auto&& pat) const -> Option<Tuple<Str, Str>>;
  auto rsplit_once(auto&& pat) const -> Option<Tuple<Str, Str>>;

 public:
  // trait: fmt::Display
  void fmt(fmt::Formatter& f) const;

  // trait: str::FromStr
  template <class T>
  auto parse() const -> Option<T> {
    if constexpr (requires { T::from_str(*this); }) {
      return T::from_str(*this);
    } else {
      return FromStr<T>::from_str(*this);
    }
  }

  // trait: serde::Serialize
  void serialize(auto& ser) const {
    ser.serialize_str(*this);
  }
};

namespace pattern {
struct SearchStep {
  enum Kind { Done = 0, Match = 1, Reject = 2 };
  Kind kind;
  Range range{0, 0};
};

template <class P>
struct Searcher {
  Str _haystack;
  P _pat;
  usize _finger = 0;
  usize _finger_back = _haystack._len;

 public:
  auto next() -> SearchStep;
  auto next_match() -> Option<Range>;
  auto next_reject() -> Option<Range>;

  auto next_match_back() -> Option<Range>;
  auto next_reject_back() -> Option<Range>;
};

struct Pattern {
  auto is_contained_in(this auto self, Str haystack) -> bool;

  auto is_prefix_of(this auto self, Str haystack) -> bool;
  auto is_suffix_of(this auto self, Str haystack) -> bool;

  auto strip_prefix_of(this auto self, Str haystack) -> Option<Str>;
  auto strip_suffix_of(this auto self, Str haystack) -> Option<Str>;
};

struct CharPattern : Pattern {
  char _ch;

 public:
  auto into_searcher(Str haystack) -> Searcher<CharPattern>;
};

struct StrPattern : Pattern {
  Str _s;

 public:
  auto into_searcher(Str haystack) -> Searcher<StrPattern>;
};

struct CharPredPattern : Pattern {
  ops::Fn<bool(char)> _pred;

 public:
  auto into_searcher(Str haystack) -> Searcher<CharPredPattern>;
};

auto from(auto&& pat) {
  if constexpr (requires { char{pat}; }) {
    return CharPattern{{}, pat};
  } else if constexpr (requires { Str{pat}; }) {
    return StrPattern{{}, pat};
  } else if constexpr (requires { pat(char{0}); }) {
    return CharPredPattern{{}, pat};
  }
}

}  // namespace pattern

auto Str::find(auto&& p) const -> Option<usize> {
  auto f_start = [](auto step) { return step._start; };
  return pattern::from(p).into_searcher(*this).next_match().map(f_start);
}

auto Str::rfind(auto&& p) const -> Option<usize> {
  auto f_start = [](auto step) { return step._start; };
  return pattern::from(p).into_searcher(*this).next_match_back().map(f_start);
}

auto Str::contains(auto&& p) const -> bool {
  return pattern::from(p).is_contained_in(*this);
}

auto Str::starts_with(auto&& p) const -> bool {
  return pattern::from(p).is_prefix_of(*this);
}

auto Str::ends_with(auto&& p) const -> bool {
  return pattern::from(p).is_suffix_of(*this);
}

auto Str::trim_start_matches(auto&& p) const -> Str {
  auto m = pattern::from(p).into_searcher(*this);
  const auto [i, _] = m.next_reject().unwrap_or({_len, _len});
  return Str{_ptr + i, _len - i};
}

auto Str::trim_end_matches(auto&& p) const -> Str {
  auto m = pattern::from(p).into_searcher(*this);
  const auto [_, b] = m.next_reject_back().unwrap_or({0, 0});
  return Str{_ptr, b};
}

auto Str::trim_matches(auto&& p) const -> Str {
  auto m = pattern::from(p).into_searcher(*this);
  const auto [i, _] = m.next_reject().unwrap_or({_len, _len});
  const auto [_, b] = m.next_reject_back().unwrap_or({0, 0});
  return Str{_ptr + i, b - i};
}

auto Str::split_once(auto&& p) const -> Option<Tuple<Str, Str>> {
  const auto [start, end] = _TRY(pattern::from(p).into_searcher(*this).next_match());
  return Tuple{Str{_ptr, start}, Str{_ptr + end, _len - end}};
}

auto Str::rsplit_once(auto&& p) const -> Option<Tuple<Str, Str>> {
  const auto [start, end] = _TRY(pattern::from(p).into_searcher(*this).next_match_back());
  return Tuple{Str{_ptr, start}, Str{_ptr + end, _len - end}};
}

}  // namespace sfc::str

namespace sfc {
using str::Str;
}  // namespace sfc
