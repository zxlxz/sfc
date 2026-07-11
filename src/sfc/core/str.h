#pragma once

#include "sfc/core/chr.h"
#include "sfc/core/slice.h"

namespace sfc::str {

struct Str {
  const char* _ptr = nullptr;
  usize _len = 0;

 public:
  constexpr Str() noexcept = default;

  constexpr Str(const char* s, usize n) noexcept : _ptr{s}, _len{n} {}

  constexpr Str(const char* s) noexcept : _ptr{s}, _len{s == nullptr ? 0 : __builtin_strlen(s)} {}

  static auto from_utf8(Slice<const u8> s) noexcept -> Str {
    return Str{ptr::cast<const char>(s._ptr), s._len};
  }

  static auto from_cstr(const char* s) noexcept -> Str {
    const auto n = s ? __builtin_strlen(s) : 0;
    return Str{s, n};
  }

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
    return Str{_ptr + ids.start, ids.len()};
  }

  auto split_at(usize mid) const noexcept -> Tuple<Str, Str> {
    const auto x = mid < _len ? mid : _len;
    return Tuple{Str{_ptr, x}, Str{_ptr + x, _len - x}};
  }

 public:
  using Iter = slice::Iter<const char>;
  auto iter() const noexcept -> slice::Iter<const char> {
    return Iter{_ptr, _len};
  }

  auto chars() const noexcept -> chr::Chars {
    return chr::Chars{ptr::cast<const u8>(_ptr), _len};
  }

 public:
  auto find(auto&& pat) const -> Option<usize>;
  auto rfind(auto&& pat) const -> Option<usize>;

  auto contains(auto&& pat) const -> bool;
  auto starts_with(auto&& pat) const -> bool;
  auto ends_with(auto&& pat) const -> bool;

  auto trim_start_matches(auto&& pat) const -> Str;
  auto trim_end_matches(auto&& pat) const -> Str;
  auto trim_matches(auto&& pat) const -> Str;

  auto trim_start() const noexcept -> Str;
  auto trim_end() const noexcept -> Str;
  auto trim() const noexcept -> Str;

  auto split_once(auto&& pat) const -> Option<Tuple<Str, Str>>;
  auto rsplit_once(auto&& pat) const -> Option<Tuple<Str, Str>>;

 public:
  // trait: ops::Eq
  constexpr auto operator==(Str other) const noexcept -> bool {
    if (_len != other._len) return false;
    if (_len == 0) return true;
    const auto ret = __builtin_memcmp(_ptr, other._ptr, _len);
    return ret == 0;
  }

  // trait: fmt::Display
  void fmt(fmt::Formatter& f) const;

  // trait: str::FromStr
  template <class T>
  auto parse() const -> Option<T>;

  // trait: serde::Serialize
  void serialize(auto& ser) const {
    ser.serialize_str(*this);
  }

  // trait: hash::Hash
  auto hash() const noexcept -> usize;
};

template <class T>
struct FromStr {
  static auto from_str(Str) -> Option<T>;
};

template <class T>
auto Str::parse() const -> Option<T> {
  if constexpr (requires { T::from_str(*this); }) {
    return T::from_str(*this);
  } else {
    return FromStr<T>::from_str(*this);
  }
}

struct SearchStep {
  enum Kind { Done = 0, Match = 1, Reject = 2 };
  Kind kind;
  ops::Range range{0, 0};
};

struct Searcher {
  auto next_match(this auto& self) -> SearchStep;
  auto next_reject(this auto& self) -> SearchStep;
  auto next_match_back(this auto& self) -> SearchStep;
  auto next_reject_back(this auto& self) -> SearchStep;
};

struct CharSearcher : Searcher {
  Str _haystack;
  char _needle;
  usize _finger = 0;
  usize _finger_back = _haystack._len;

 public:
  auto next() -> SearchStep;
  auto next_back() -> SearchStep;
};

struct StrSearcher : Searcher {
  Str _haystack;
  Str _needle;
  usize _finger = 0;
  usize _finger_back = _haystack._len;

 public:
  auto next() -> SearchStep;
  auto next_back() -> SearchStep;
};

struct CharPredicateSearcher : Searcher {
  Str _haystack;
  ops::Dyn<bool(char)> _pred;
  usize _finger = 0;
  usize _finger_back = _haystack._len;

 public:
  auto next() -> SearchStep;
  auto next_back() -> SearchStep;
};

struct Pattern {
  static auto into_searcher(auto&& self, Str haystack) {
    if constexpr (requires { char{self}; }) {
      return CharSearcher{{}, haystack, self};
    } else if constexpr (requires { Str{self}; }) {
      return StrSearcher{{}, haystack, self};
    } else if constexpr (requires { self(char{0}); }) {
      return CharPredicateSearcher{{}, haystack, mem::move(self)};
    }
  }
};

auto Str::find(auto&& pat) const -> Option<usize> {
  if (_len == 0) {
    return {};
  }

  auto s = Pattern::into_searcher(pat, *this);
  const auto [kind, step] = s.next_match();
  if (kind != SearchStep::Match) {
    return {};
  }
  return step.start;
}

auto Str::rfind(auto&& pat) const -> Option<usize> {
  if (_len == 0) {
    return {};
  }

  auto s = Pattern::into_searcher(pat, *this);
  const auto [kind, step] = s.next_match_back();
  if (kind != SearchStep::Match) {
    return {};
  }
  return step.start;
}

auto Str::contains(auto&& pat) const -> bool {
  if (_len == 0) {
    return false;
  }

  auto s = Pattern::into_searcher(pat, *this);
  const auto [kind, step] = s.next_match();
  return kind == SearchStep::Match;
}

auto Str::starts_with(auto&& pat) const -> bool {
  if (_len == 0) {
    return false;
  }

  auto ss = Pattern::into_searcher(pat, *this);
  const auto step = ss.next();
  return step.kind == SearchStep::Match;
}

auto Str::ends_with(auto&& pat) const -> bool {
  if (_len == 0) {
    return false;
  }

  auto searcher = Pattern::into_searcher(pat, *this);
  const auto step = searcher.next_back();
  return step.kind == SearchStep::Match;
}

auto Str::trim_start_matches(auto&& pat) const -> Str {
  if (_len == 0) {
    return {};
  }

  auto s = Pattern::into_searcher(pat, *this);
  auto i = _len;
  if (auto [kind, step] = s.next_reject(); kind != SearchStep::Done) {
    i = step.start;
  }
  return Str{_ptr + i, _len - i};
}

auto Str::trim_end_matches(auto&& pat) const -> Str {
  if (_len == 0) {
    return {};
  }

  auto s = Pattern::into_searcher(pat, *this);
  auto j = usize{0U};
  if (auto [kind, step] = s.next_reject_back(); kind != SearchStep::Done) {
    j = step.end;
  }
  return Str{_ptr, j};
}

auto Str::trim_matches(auto&& pat) const -> Str {
  if (_len == 0) {
    return {};
  }

  auto s = Pattern::into_searcher(pat, *this);
  auto i = usize{0U};
  auto j = usize{0U};
  if (auto [kind, step] = s.next_reject(); kind != SearchStep::Done) {
    i = step.start;
    j = step.end;
  }
  if (auto [kind, step] = s.next_reject_back(); kind != SearchStep::Done) {
    j = step.end;
  }
  return Str{_ptr + i, j - i};
}

auto Str::split_once(auto&& pat) const -> Option<Tuple<Str, Str>> {
  if (_len == 0) {
    return {};
  }

  auto s = Pattern::into_searcher(pat, *this);
  const auto [kind, range] = s.next_match();
  if (kind != SearchStep::Match) {
    return {};
  }
  const auto [i, j] = range;
  return Tuple{Str{_ptr, i}, Str{_ptr + j, _len - j}};
}

auto Str::rsplit_once(auto&& pat) const -> Option<Tuple<Str, Str>> {
  if (_len == 0) {
    return {};
  }

  auto s = Pattern::into_searcher(pat, *this);
  auto [kind, step] = s.next_match_back();
  if (kind != SearchStep::Match) {
    return {};
  }
  const auto [i, j] = step;
  return Tuple{Str{_ptr, i}, Str{_ptr + j, _len - j}};
}

}  // namespace sfc::str

namespace sfc {
using str::Str;
}  // namespace sfc
