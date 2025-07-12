#pragma once

#include "sfc/core/chr.h"
#include "sfc/core/slice.h"

namespace sfc::str {

template <class T>
struct FromStr;

struct Str {
  const char* _ptr = nullptr;
  usize       _len = 0;

 public:
  constexpr Str() = default;

  constexpr Str(const char* p, usize n) noexcept : _ptr{p}, _len{n} {}

  template <usize N>
  constexpr Str(const char (&s)[N]) noexcept : _ptr{s}, _len{N - 1} {}

  static constexpr auto from_cstr(const char* p) -> Str {
    return Str{p, p ? __builtin_strlen(p) : 0};
  }

  static constexpr auto from_chars(slice::Slice<const char> s) -> Str {
    return Str{s._ptr, s._len};
  }

  static constexpr auto from_bytes(slice::Slice<const u8> s) -> Str {
    const auto p = reinterpret_cast<const char*>(s._ptr);
    return Str{p, s._len};
  }

  auto as_ptr() const -> const char* {
    return _ptr;
  }

  auto is_empty() const -> bool {
    return _len == 0;
  }

  auto len() const -> usize {
    return _len;
  }

  explicit operator bool() const {
    return _len != 0;
  }

  auto as_chars() const -> slice::Slice<const char> {
    return {_ptr, _len};
  }

  auto as_bytes() const -> slice::Slice<const u8> {
    return {reinterpret_cast<const u8*>(_ptr), _len};
  }

  auto as_str() const -> Str {
    return *this;
  }

 public:
  auto get_unchecked(usize idx) const -> char {
    return _ptr[idx];
  }

  auto operator[](usize idx) const -> char {
    return idx < _len ? _ptr[idx] : '\0';
  }

  auto operator[](Range ids) const -> Str {
    const auto end = cmp::min(ids._end, _len);
    const auto pos = cmp::min(ids._start, end);
    return {_ptr + pos, end - pos};
  }

  auto split_at(usize mid) const -> tuple::Tuple<Str, Str> {
    const auto x = cmp::min(mid, _len);
    return tuple::Tuple{Str{_ptr, x}, Str{_ptr + x, _len - x}};
  }

  auto iter() const -> slice::Iter<const char> {
    return this->as_chars().iter();
  }

  auto operator==(Str other) const -> bool {
    if (_len != other._len) {
      return false;
    }

    const auto ret = __builtin_memcmp(_ptr, other._ptr, _len);
    return ret == 0;
  }

 public:
  auto find(auto&& p) const -> option::Option<usize>;
  auto rfind(auto&& p) const -> option::Option<usize>;

  auto contains(auto&& p) const -> bool;
  auto starts_with(auto&& p) const -> bool;
  auto ends_with(auto&& p) const -> bool;

  auto trim_start_matches(auto&& p) const -> Str;
  auto trim_end_matches(auto&& p) const -> Str;
  auto trim_matches(auto&& p) const -> Str;

  auto split(auto&& p) const;

  auto trim_start() const -> Str {
    return this->trim_start_matches(chr::is_whitespace);
  }

  auto trim_end() const -> Str {
    return this->trim_end_matches(chr::is_whitespace);
  }

  auto trim() const -> Str {
    return this->trim_matches(chr::is_whitespace);
  }

  template <class T>
  auto parse() const -> option::Option<T> {
    if constexpr (requires { T::from_str(*this); }) {
      return T::from_str(*this);
    } else {
      return FromStr<T>::from_str(*this);
    }
  }

  void fmt(auto& f) const {
    f.pad(*this);
  }
};

struct SearchStep {
  enum Kind { Match, Reject, Done };
  Kind  kind;
  Range range = {0UL, 0UL};

  explicit operator bool() const {
    return kind != Kind::Done;
  }
};

template <class P>
struct CharPredSercher {
  Str   _haystack;
  P     _pred;
  usize _finger      = 0U;
  usize _finger_back = _haystack.len();

 public:
  auto next() -> SearchStep {
    if (_finger >= _finger_back) {
      return {.kind = SearchStep::Done};
    }

    const auto ch = _haystack.get_unchecked(_finger);
    const auto mm = _pred(ch) ? SearchStep::Match : SearchStep::Reject;
    const auto ss = SearchStep{mm, {_finger, _finger + 1}};
    _finger += 1;
    return ss;
  }

  auto next_back() -> SearchStep {
    if (_finger >= _finger_back) {
      return {.kind = SearchStep::Done};
    }

    const auto ch = _haystack.get_unchecked(_finger_back);
    const auto mm = _pred(ch) ? SearchStep::Match : SearchStep::Reject;
    const auto ss = SearchStep{mm, {_finger_back - 1, _finger_back}};
    _finger_back -= 1;
    return ss;
  }

  auto next_match() -> Option<Range> {
    while (const auto ss = this->next()) {
      if (ss.kind == SearchStep::Match) {
        return Option{ss.range};
      }
    }
    return {};
  }

  auto next_match_back() -> Option<Range> {
    while (const auto ss = this->next_back()) {
      if (ss.kind == SearchStep::Match) {
        return Option{ss.range};
      }
    }
    return {};
  }

  auto next_reject() -> Option<Range> {
    while (const auto ss = this->next()) {
      if (ss.kind == SearchStep::Reject) {
        return Option{ss.range};
      }
    }
    return {};
  }

  auto next_reject_back() -> Option<Range> {
    while (const auto ss = this->next_back()) {
      if (ss.kind == SearchStep::Reject) {
        return Option{ss.range};
      }
    }
    return {};
  }
};

struct CharSearcher {
  Str   _haystack;
  char  _needle;
  usize _finger      = 0U;
  usize _finger_back = _haystack.len();

 public:
  auto next() -> SearchStep {
    if (_finger >= _finger_back) {
      return {.kind = SearchStep::Done};
    }

    const auto ch  = _haystack.get_unchecked(_finger);
    const auto mm  = ch == _needle ? SearchStep::Match : SearchStep::Reject;
    const auto res = SearchStep{.kind = mm, .range = {_finger, _finger + 1}};
    _finger += 1;
    return res;
  }

  auto next_back() -> SearchStep {
    if (_finger >= _finger_back) {
      return {.kind = SearchStep::Done};
    }

    const auto ch  = _haystack.get_unchecked(_finger_back - 1);
    const auto mm  = ch == _needle ? SearchStep::Match : SearchStep::Reject;
    const auto res = SearchStep{.kind = mm, .range = {_finger_back - 1, _finger_back}};
    _finger_back -= 1;
    return res;
  }

  auto next_match() -> Option<Range> {
    if (_finger >= _finger_back) {
      return {};
    }

    if (auto p = __builtin_memchr(_haystack._ptr, _needle, _finger_back - _finger)) {
      const auto n = static_cast<usize>(static_cast<const char*>(p) - _haystack._ptr);
      return Option{Range{n, n + 1}};
    }

    return {};
  }

  auto next_match_back() -> Option<Range> {
    while (const auto ss = this->next_back()) {
      if (ss.kind == SearchStep::Match) {
        return Option{ss.range};
      }
    }
    return {};
  }
};

struct StrSearcher {
  Str   _haystack;
  Str   _needle;
  usize _finger      = 0;
  usize _finger_back = num::saturating_sub(_haystack.len() + 1, _needle.len());

 public:
  auto next() -> SearchStep {
    if (_finger >= _finger_back) {
      return {.kind = SearchStep::Done};
    }

    const auto ss  = _haystack[{_finger, _finger + _needle._len}];
    const auto mm  = ss == _needle ? SearchStep::Match : SearchStep::Reject;
    const auto res = SearchStep{.kind = mm, .range = {_finger, _finger + _needle._len}};
    _finger += mm == SearchStep::Match ? _needle._len : 1U;
    return res;
  }

  auto next_back() -> SearchStep {
    if (_finger >= _finger_back) {
      return {.kind = SearchStep::Done};
    }

    const auto ch  = _haystack[{_finger_back - _needle._len, _finger_back}];
    const auto mm  = ch == _needle ? SearchStep::Match : SearchStep::Reject;
    const auto res = SearchStep{.kind = mm, .range = {_finger_back, _finger_back + 1}};
    _finger_back -= mm == SearchStep::Match ? _needle._len : 1U;
    return res;
  }
};

template <class P>
struct Pattern {
  P _pred;

  auto into_searcher(Str s) && -> CharPredSercher<P> {
    return CharPredSercher<P>{s, static_cast<P&&>(_pred)};
  }
};

template <>
struct Pattern<char> {
  char _needle;

  auto into_searcher(Str s) const -> CharSearcher {
    return CharSearcher{._haystack = s, ._needle = _needle};
  }
};

template <>
struct Pattern<Str> {
  Str _needle;

  auto into_searcher(Str s) const -> StrSearcher {
    return StrSearcher{._haystack = s, ._needle = _needle};
  }
};

template <class P>
Pattern(P) -> Pattern<P>;

Pattern(char) -> Pattern<char>;

Pattern(Str) -> Pattern<Str>;

template <usize N>
Pattern(const char (&)[N]) -> Pattern<Str>;

template <usize N>
Pattern(char (&)[N]) -> Pattern<Str>;

template <class S>
struct Split : iter::Iterator<Split<S>, Str> {
  S _searcher;

 public:
  auto next() -> Option<Str> {
    const auto cur_pos = _searcher._finger;
    _searcher.next();
  }
};

template <class P>
auto Str::find(P&& p) const -> Option<usize> {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  auto t = s.next_match();
  return t.map([](auto ids) { return ids._start; });
}

template <class P>
auto Str::rfind(P&& p) const -> Option<usize> {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  auto t = s.next_match_back();
  return t.map([](auto ids) { return ids._start; });
}

template <class P>
auto Str::contains(P&& p) const -> bool {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  auto t = s.next_match();
  return t;
}

template <class P>
auto Str::starts_with(P&& p) const -> bool {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  auto t = s.next();
  return t.kind == SearchStep::Match;
}

template <class P>
auto Str::ends_with(P&& p) const -> bool {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  auto t = s.next_back();
  return t.kind == SearchStep::Match;
}

template <class P>
auto Str::trim_start_matches(P&& p) const -> Str {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  auto i = s.next_reject().unwrap_or({0U, 0U});
  return (*this)[{i._start, _len}];
}

template <class P>
auto Str::trim_end_matches(P&& p) const -> Str {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  auto j = s.next_reject_back().unwrap_or({_len, _len});
  return (*this)[{0, j._start}];
}

template <class P>
auto Str::trim_matches(P&& p) const -> Str {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  auto i = s.next_reject().unwrap_or({0U, 0U});
  auto j = s.next_reject_back().unwrap_or({_len, _len});
  return (*this)[{i._start, j._start}];
}

template <class P>
auto Str::split(P&& p) const {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  return Split{*this, s};
}

template <class T>
struct FromStr {
  static auto from_str(Str) -> option::Option<T>;
};

}  // namespace sfc::str
