#pragma once

#include "mod.h"

namespace sfc::str {

namespace pattern {

struct SearchStep {
  enum Type { Match, Reject, Done };
  Type type;
  Range<> ids = {0UL, 0UL};

  operator bool() const {
    return type != Done;
  }
};

template <class P>
struct CharPredSercher {
  Str _haystack;
  P _pred;
  usize _finger = 0U;
  usize _finger_back = _haystack.len();

 public:
  auto next() -> SearchStep {
    if (_finger >= _finger_back) {
      return {SearchStep::Done};
    }

    const auto ch = _haystack.get_unchecked(_finger);
    const auto mm = _pred(ch) ? SearchStep::Match : SearchStep::Reject;
    const auto ss = SearchStep{mm, {_finger, _finger + 1}};
    _finger += 1;
    return ss;
  }

  auto next_back() -> SearchStep {
    if (_finger >= _finger_back) {
      return {SearchStep::Done};
    }

    const auto ch = _haystack.get_unchecked(_finger_back);
    const auto mm = _pred(ch) ? SearchStep::Match : SearchStep::Reject;
    const auto ss = SearchStep{mm, {_finger_back - 1, _finger_back}};
    _finger_back -= 1;
    return ss;
  }

  auto next_match() -> Option<Range<>> {
    for (; const auto ss = this->next();) {
      if (ss.type == SearchStep::Match) {
        return ss.ids;
      }
    }
    return {};
  }

  auto next_match_back() -> Option<Range<>> {
    for (; const auto ss = this->next_back();) {
      if (ss.type == SearchStep::Match) {
        return ss.ids;
      }
    }
    return {};
  }

  auto next_reject() -> Option<Range<>> {
    for (; const auto ss = this->next();) {
      if (ss.type == SearchStep::Reject) {
        return ss.ids;
      }
    }
    return {};
  }

  auto next_reject_back() -> Option<Range<>> {
    for (; const auto ss = this->next_back();) {
      if (ss.type == SearchStep::Reject) {
        return ss.ids;
      }
    }
    return {};
  }
};

struct CharSearcher {
  Str _haystack;
  char _needle;
  usize _finger = 0U;
  usize _finger_back = _haystack.len();

 public:
  auto next() -> SearchStep {
    if (_finger >= _finger_back) {
      return {SearchStep::Done};
    }

    const auto ch = _haystack.get_unchecked(_finger);
    const auto mm = ch == _needle ? SearchStep::Match : SearchStep::Reject;
    const auto res = SearchStep{mm, {_finger, _finger + 1}};
    _finger += 1;
    return res;
  }

  auto next_back() -> SearchStep {
    if (_finger >= _finger_back) {
      return {SearchStep::Done};
    }

    const auto ch = _haystack.get_unchecked(_finger_back - 1);
    const auto mm = ch == _needle ? SearchStep::Match : SearchStep::Reject;
    const auto res = SearchStep{mm, {_finger_back - 1, _finger_back}};
    _finger_back -= 1;
    return res;
  }

  auto next_match() -> Option<Range<>> {
    if (_finger >= _finger_back) return {};

    if (auto p = __builtin_memchr(_haystack._ptr, _needle, _finger_back - _finger)) {
      const auto n = static_cast<usize>(static_cast<const char*>(p) - _haystack._ptr);
      return Range<>{n, n + 1};
    }

    return {};
  }

  auto next_match_back() -> Option<Range<>> {
    for (; const auto ss = this->next_back();) {
      if (ss.type == SearchStep::Match) {
        return ss.ids;
      }
    }
    return {};
  }
};

struct StrSearcher {
  Str _haystack;
  Str _needle;
  usize _finger = 0;
  usize _finger_back = num::saturating_sub(_haystack.len() + 1, _needle.len());

 public:
  auto next() -> SearchStep;
  auto next_back() -> SearchStep;
};

template <class P>
struct Pattern {
  P _pred;

 public:
  auto into_searcher(Str s) && -> CharPredSercher<P> {
    return {s, static_cast<P&&>(_pred)};
  }
};

template <>
struct Pattern<char> {
  char _needle;

 public:
  auto into_searcher(Str s) const -> CharSearcher {
    return {s, _needle};
  }
};

template <>
struct Pattern<Str> {
  Str _needle;

 public:
  auto into_searcher(Str s) const -> StrSearcher {
    return {s, _needle};
  }
};

template <class P>
Pattern(P) -> Pattern<P>;

Pattern(char) -> Pattern<char>;
Pattern(const char*) -> Pattern<Str>;
Pattern(Str) -> Pattern<Str>;

template <class P>
auto make_pattern(P p) -> Pattern<P> {
  return {mem::move(p)};
}

inline auto make_pattern(char ch) -> Pattern<char> {
  return {ch};
}

inline auto make_pattern(const char* s) -> Pattern<Str> {
  return {s};
}

inline auto make_pattern(Str s) -> Pattern<Str> {
  return {s};
}

}  // namespace pattern

template <class P>
auto Str::find(P&& p) const -> Option<usize> {
  auto s = pattern::make_pattern(static_cast<P&&>(p)).into_searcher(*this);
  auto t = s.next_match();
  return t.map([](auto ids) { return ids._start; });
}

template <class P>
auto Str::rfind(P&& p) const -> Option<usize> {
  auto s = pattern::make_pattern(static_cast<P&&>(p)).into_searcher(*this);
  auto t = s.next_match_back();
  return t.map([](auto ids) { return ids._start; });
}

template <class P>
auto Str::contains(P&& p) const -> bool {
  auto s = pattern::make_pattern(static_cast<P&&>(p)).into_searcher(*this);
  auto t = s.next_match();
  return t;
}

template <class P>
auto Str::starts_with(P&& p) const -> bool {
  auto s = pattern::make_pattern(static_cast<P&&>(p)).into_searcher(*this);
  auto t = s.next();
  return t.type == pattern::SearchStep::Match;
}

template <class P>
auto Str::ends_with(P&& p) const -> bool {
  auto s = pattern::make_pattern(static_cast<P&&>(p)).into_searcher(*this);
  auto t = s.next_back();
  return t.type == pattern::SearchStep::Match;
}

template <class P>
auto Str::trim_start_matches(P&& p) const -> Str {
  auto s = pattern::make_pattern(static_cast<P&&>(p)).into_searcher(*this);
  auto i = s.next_reject().unwrap_or({0U, 0U});
  return this->get_unchecked({i._start, _len});
}

template <class P>
auto Str::trim_end_matches(P&& p) const -> Str {
  auto s = pattern::make_pattern(static_cast<P&&>(p)).into_searcher(*this);
  auto j = s.next_reject_back().unwrap_or({_len, _len});
  return this->get_unchecked({0, j._start});
}

template <class P>
auto Str::trim_matches(P&& p) const -> Str {
  auto s = pattern::make_pattern(static_cast<P&&>(p)).into_searcher(*this);
  auto i = s.next_reject().unwrap_or({0U, 0U});
  auto j = s.next_reject_back().unwrap_or({_len, _len});
  return this->get_unchecked({i._start, j._start});
}

}  // namespace sfc::str
