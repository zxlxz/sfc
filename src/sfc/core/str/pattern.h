#pragma once

#include "sfc/core/str/mod.h"

namespace sfc::str {

struct SearchStep {
  enum Kind { Match, Reject, Done };
  Kind  kind;
  Range range = {0UL, 0UL};

  operator bool() const {
    return kind != Kind::Done;
  }
};

template <class P>
struct CharPredSercher {
  Str   _haystack;
  P     _pred;
  usize _finger = 0U;
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
  usize _finger = 0U;
  usize _finger_back = _haystack.len();

 public:
  auto next() -> SearchStep {
    if (_finger >= _finger_back) {
      return {.kind = SearchStep::Done};
    }

    const auto ch = _haystack.get_unchecked(_finger);
    const auto mm = ch == _needle ? SearchStep::Match : SearchStep::Reject;
    const auto res = SearchStep{.kind = mm, .range = {_finger, _finger + 1}};
    _finger += 1;
    return res;
  }

  auto next_back() -> SearchStep {
    if (_finger >= _finger_back) {
      return {.kind = SearchStep::Done};
    }

    const auto ch = _haystack.get_unchecked(_finger_back - 1);
    const auto mm = ch == _needle ? SearchStep::Match : SearchStep::Reject;
    const auto res = SearchStep{.kind = mm, .range = {_finger_back - 1, _finger_back}};
    _finger_back -= 1;
    return res;
  }

  auto next_match() -> Option<Range> {
    if (_finger >= _finger_back)
      return {};

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
  usize _finger = 0;
  usize _finger_back = num::saturating_sub(_haystack.len() + 1, _needle.len());

 public:
  auto next() -> SearchStep {
    if (_finger >= _finger_back) {
      return {.kind = SearchStep::Done};
    }

    const auto ss = _haystack.get_unchecked({_finger, _finger + _needle._len});
    const auto mm = ss == _needle ? SearchStep::Match : SearchStep::Reject;
    const auto res = SearchStep{.kind = mm, .range = {_finger, _finger + _needle._len}};
    _finger += mm == SearchStep::Match ? _needle._len : 1U;
    return res;
  }

  auto next_back() -> SearchStep {
    if (_finger >= _finger_back) {
      return {.kind = SearchStep::Done};
    }

    const auto ch = _haystack.get_unchecked({_finger_back - _needle._len, _finger_back});
    const auto mm = ch == _needle ? SearchStep::Match : SearchStep::Reject;
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
struct Split {
  using Item = Str;
  using Searcher = S;
  Searcher _searcher;

 public:
  auto next() -> Option<Str> {
    const auto cur_pos = _searcher._finger;
    _searcher.next();
  }

  auto next_back() -> Option<Str> {}

  auto operator->() -> iter::Iterator<Split>* {
    return static_cast<iter::Iterator<Split>*>(this);
  }
};

template <class P>
auto Str::find(P&& p) const -> Option<usize> {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  auto t = s.next_match();
  return t.map([](auto ids) {
    return ids._start;
  });
}

template <class P>
auto Str::rfind(P&& p) const -> Option<usize> {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  auto t = s.next_match_back();
  return t.map([](auto ids) {
    return ids._start;
  });
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
  return this->get_unchecked({i._start, _len});
}

template <class P>
auto Str::trim_end_matches(P&& p) const -> Str {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  auto j = s.next_reject_back().unwrap_or({_len, _len});
  return this->get_unchecked({0, j._start});
}

template <class P>
auto Str::trim_matches(P&& p) const -> Str {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  auto i = s.next_reject().unwrap_or({0U, 0U});
  auto j = s.next_reject_back().unwrap_or({_len, _len});
  return this->get_unchecked({i._start, j._start});
}

template <class P>
auto Str::split(P&& p) const {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  return Split{*this, s};
}

}  // namespace sfc::str
