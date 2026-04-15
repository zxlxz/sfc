#pragma once

#include "sfc/core/str/mod.h"

namespace sfc::str::pattern {

struct SearchStep {
  enum Type { Done = 0, Match = 1, Reject = 2 };
  Type _type;
  u32 _pos;
  u32 _end;

 public:
  [[gnu::always_inline]] operator bool() const {
    return _type != Done;
  }

  [[gnu::always_inline]] auto pos() const -> Option<usize> {
    return _type ? Option<usize>{_pos} : Option<usize>{};
  }
};

struct Searcher {
  Str _haystack;

 public:
  auto next_match(this auto& self) -> SearchStep {
    while (true) {
      const auto step = self.next();
      switch (step._type) {
        case SearchStep::Done:  return {SearchStep::Done, 0, 0};
        case SearchStep::Match: return step;
        default:                break;
      }
    }
  }

  auto next_reject(this auto& self) -> SearchStep {
    while (true) {
      const auto step = self.next();
      switch (step._type) {
        case SearchStep::Done:   return {SearchStep::Done, 0, 0};
        case SearchStep::Reject: return step;
        default:                 break;
      }
    }
  }

  auto next_match_back(this auto& self) -> SearchStep {
    while (true) {
      const auto step = self.next_back();
      switch (step._type) {
        case SearchStep::Done:  return {SearchStep::Done, 0, 0};
        case SearchStep::Match: return step;
        default:                break;
      }
    }
  }

  auto next_reject_back(this auto& self) -> SearchStep {
    while (true) {
      const auto step = self.next_back();
      switch (step._type) {
        case SearchStep::Done:   return {SearchStep::Done, 0, 0};
        case SearchStep::Reject: return step;
        default:                 break;
      }
    }
  }
};

struct CharSearcher : Searcher {
  char _needle;
  u32 _finger = 0;
  u32 _finger_back = static_cast<u32>(_haystack._len);

 public:
  auto next() -> SearchStep {
    if (_finger >= _haystack._len) {
      return {SearchStep::Done, 0, 0};
    }

    const auto ch = _haystack[_finger++];
    if (ch == _needle) {
      return {SearchStep::Match, _finger - 1, _finger};
    } else {
      return {SearchStep::Reject, _finger - 1, _finger};
    }
  }

  auto next_back() -> SearchStep {
    if (_finger_back == 0) {
      return {SearchStep::Done, 0, 0};
    }

    const auto ch = _haystack[_finger_back - 1];
    if (ch == _needle) {
      _finger_back -= 1;
      return {SearchStep::Match, _finger_back, _finger_back + 1};
    } else {
      _finger_back -= 1;
      return {SearchStep::Reject, _finger_back, _finger_back + 1};
    }
  }
};

template <class F>
struct CharPredicateSearcher : Searcher {
  F& _match;
  u32 _finger = 0;
  u32 _finger_back = static_cast<u32>(_haystack._len);

 public:
  auto next() -> SearchStep {
    if (_finger >= _haystack._len) {
      return {SearchStep::Done, 0, 0};
    }

    const auto ch = _haystack[_finger++];
    if (_match(ch)) {
      return {SearchStep::Match, _finger - 1, _finger};
    } else {
      return {SearchStep::Reject, _finger - 1, _finger};
    }
  }

  auto next_back() -> SearchStep {
    if (_finger_back == 0) {
      return {SearchStep::Done, 0, 0};
    }

    const auto ch = _haystack[--_finger_back];
    if (_match(ch)) {
      return {SearchStep::Match, _finger_back, _finger_back + 1};
    } else {
      return {SearchStep::Reject, _finger_back, _finger_back + 1};
    }
  }
};

struct StrSearcher : Searcher {
  Str _needle;
  u32 _finger = 0;
  u32 _finger_back = static_cast<u32>(_haystack._len);

  auto match() const -> bool {
    if (_needle._len == 0) return true;
    if (_finger + _needle._len > _haystack._len) return false;

    const auto p = _haystack._ptr + _finger;
    return __builtin_memcmp(p, _needle._ptr, _needle._len) == 0;
  }

  auto match_back() const -> bool {
    if (_needle._len == 0) return true;
    if (_finger_back < _needle._len) return false;

    const auto p = _haystack._ptr + _finger_back - _needle._len;
    return __builtin_memcmp(p, _needle._ptr, _needle._len) == 0;
  }

  auto next() -> SearchStep {
    if (_finger >= _haystack._len) {
      return {SearchStep::Done, 0, 0};
    }

    const auto old_finger = _finger;
    if (this->match()) {
      _finger += static_cast<u32>(_needle._len);
      return {SearchStep::Match, old_finger, _finger};
    } else {
      if (_finger + _needle._len < _haystack._len) {
        _finger += 1;
      } else {
        _finger = static_cast<u32>(_haystack._len);
      }
      return {SearchStep::Reject, old_finger, _finger};
    }
  }

  auto next_back() -> SearchStep {
    if (_finger_back == 0) {
      return {SearchStep::Done, 0, 0};
    }

    const auto old_finger_back = _finger_back;
    if (this->match_back()) {
      _finger_back -= static_cast<u32>(_needle._len);
      return {SearchStep::Match, _finger_back, old_finger_back};
    } else {
      if (_finger_back >= _needle._len) {
        _finger_back -= static_cast<u32>(_needle._len);
      } else {
        _finger_back = 0;
      }
      return {SearchStep::Reject, _finger_back, old_finger_back};
    }
  }
};

struct Pattern {
  static auto into_searcher(auto& self, Str haystack) {
    if constexpr (requires { char{self}; }) {
      return CharSearcher{{haystack}, self};
    } else if constexpr (requires { Str{self}; }) {
      return StrSearcher{{haystack}, self};
    } else if constexpr (requires { self(char{0}); }) {
      return CharPredicateSearcher{{haystack}, self};
    }
  }
};

}  // namespace sfc::str::pattern

namespace sfc::str {

auto Str::find(auto&& pat) const -> Option<usize> {
  if (_len == 0) {
    return {};
  }

  auto s = pattern::Pattern::into_searcher(pat, *this);
  return s.next_match().pos();
}

auto Str::rfind(auto&& pat) const -> Option<usize> {
  if (_len == 0) {
    return {};
  }

  auto s = pattern::Pattern::into_searcher(pat, *this);
  return s.next_match_back().pos();
}

auto Str::contains(auto&& pat) const -> bool {
  if (_len == 0) {
    return false;
  }

  auto s = pattern::Pattern::into_searcher(pat, *this);
  return s.next_match()._type == pattern::SearchStep::Match;
}

auto Str::starts_with(auto&& pat) const -> bool {
  if (_len == 0) {
    return false;
  }

  auto s = pattern::Pattern::into_searcher(pat, *this);
  return s.next()._type == pattern::SearchStep::Match;
}

auto Str::ends_with(auto&& pat) const -> bool {
  if (_len == 0) {
    return false;
  }

  auto s = pattern::Pattern::into_searcher(pat, *this);
  return s.next_back()._type == pattern::SearchStep::Match;
}

auto Str::trim_start_matches(auto&& pat) const -> Str {
  if (_len == 0) {
    return {};
  }

  auto s = pattern::Pattern::into_searcher(pat, *this);
  auto i = _len;
  if (auto m = s.next_reject()) {
    i = m._pos;
  }
  return Str{_ptr + i, _len - i};
}

auto Str::trim_end_matches(auto&& pat) const -> Str {
  if (_len == 0) {
    return {};
  }

  auto s = pattern::Pattern::into_searcher(pat, *this);
  auto j = 0U;
  if (auto m = s.next_reject_back()) {
    j = m._end;
  }
  return Str{_ptr, j};
}

auto Str::trim_matches(auto&& pat) const -> Str {
  if (_len == 0) {
    return {};
  }

  auto s = pattern::Pattern::into_searcher(pat, *this);
  auto i = 0U;
  auto j = 0U;
  if (auto m = s.next_reject()) {
    i = m._pos;
    j = m._end;
  }
  if (auto m = s.next_reject_back()) {
    j = m._end;
  }
  return Str{_ptr + i, j - i};
}

}  // namespace sfc::str
