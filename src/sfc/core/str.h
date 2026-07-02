#pragma once

#include "sfc/core/chr.h"
#include "sfc/core/slice.h"

namespace sfc::str {

struct Str {
  const char* _ptr = nullptr;
  usize _len = 0;

 public:
  [[gnu::always_inline]] constexpr Str() noexcept = default;

  [[gnu::always_inline]] constexpr Str(const char* s, usize n) noexcept : _ptr{s}, _len{n} {}

  template <usize N>
  [[gnu::always_inline]] constexpr Str(const char (&s)[N]) noexcept : _ptr{s}, _len{N - 1} {}

  [[gnu::always_inline]] static auto from_utf8(Slice<const u8> s) noexcept -> Str {
    return Str{ptr::cast<const char>(s._ptr), s._len};
  }

  [[gnu::always_inline]] static auto from_cstr(const char* s) noexcept -> Str {
    const auto n = s ? __builtin_strlen(s) : 0;
    return Str{s, n};
  }

  [[gnu::always_inline]] constexpr auto ptr() const noexcept -> const char* {
    return _ptr;
  }

  [[gnu::always_inline]] constexpr auto len() const noexcept -> usize {
    return _len;
  }

  [[gnu::always_inline]] constexpr auto as_ptr() const noexcept -> const char* {
    return _ptr;
  }

  [[gnu::always_inline]] constexpr auto as_str() const noexcept -> Str {
    return *this;
  }

  [[gnu::always_inline]] constexpr auto data() const noexcept -> const char* {
    return _ptr;
  }

  [[gnu::always_inline]] constexpr auto size() const noexcept -> usize {
    return _len;
  }

  [[gnu::always_inline]] constexpr auto is_empty() const noexcept -> bool {
    return _len == 0;
  }

  [[gnu::always_inline]] auto as_bytes() const noexcept -> Slice<const u8> {
    return {ptr::cast<const u8>(_ptr), _len};
  }

 public:
  [[gnu::always_inline]] constexpr auto operator[](usize idx) const noexcept -> char {
    if (!_ptr || idx >= _len) return 0;
    return _ptr[idx];
  }

  [[gnu::always_inline]] constexpr auto operator[](Range ids) const noexcept -> Str {
    const auto start = ids.start < _len ? ids.start : _len;
    const auto end = ids.end < _len ? ids.end : _len;
    return Str{_ptr + start, start < end ? end - start : 0U};
  }

  [[gnu::always_inline]] auto split_at(usize mid) const noexcept -> Tuple<Str, Str> {
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
  void fmt(auto& f) const {
    const auto dbg_mode = f._spec._type == 's' || f._spec._type == '?';
    if (dbg_mode) {
      f.write_char('"');
      f.write_str(*this);
      f.write_char('"');
    } else {
      f.pad(*this);
    }
  }

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
  enum Type { Done = 0, Match = 1, Reject = 2 };
  Type _type;
  usize _pos;
  usize _end;

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
  usize _finger = 0;
  usize _finger_back = _haystack._len;

 public:
  auto next() -> SearchStep;
  auto next_back() -> SearchStep;
};

struct StrSearcher : Searcher {
  Str _needle;
  usize _finger = 0;
  usize _finger_back = _haystack._len;

 public:
  auto match() const -> bool;
  auto match_back() const -> bool;
  auto next() -> SearchStep;
  auto next_back() -> SearchStep;
};

template <class F>
struct CharPredicateSearcher : Searcher {
  F _pred;
  usize _finger = 0;
  usize _finger_back = _haystack._len;

 public:
  auto next() -> SearchStep {
    if (_finger >= _haystack._len) {
      return {SearchStep::Done, 0, 0};
    }

    const auto ch = _haystack[_finger++];
    if (_pred(ch)) {
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
    if (_pred(ch)) {
      return {SearchStep::Match, _finger_back, _finger_back + 1};
    } else {
      return {SearchStep::Reject, _finger_back, _finger_back + 1};
    }
  }
};

struct Pattern {
  static auto into_searcher(auto&& self, Str haystack) {
    if constexpr (requires { char{self}; }) {
      return CharSearcher{{haystack}, self};
    } else if constexpr (requires { Str{self}; }) {
      return StrSearcher{{haystack}, self};
    } else if constexpr (requires { self(char{0}); }) {
      return CharPredicateSearcher{{haystack}, mem::move(self)};
    }
  }
};

auto Str::find(auto&& pat) const -> Option<usize> {
  if (_len == 0) {
    return {};
  }

  auto s = Pattern::into_searcher(pat, *this);
  auto m = s.next_match();
  return m.pos();
}

auto Str::rfind(auto&& pat) const -> Option<usize> {
  if (_len == 0) {
    return {};
  }

  auto s = Pattern::into_searcher(pat, *this);
  auto m = s.next_match_back();
  return m.pos();
}

auto Str::contains(auto&& pat) const -> bool {
  if (_len == 0) {
    return false;
  }

  auto s = Pattern::into_searcher(pat, *this);
  auto m = s.next_match();
  return m._type == SearchStep::Match;
}

auto Str::starts_with(auto&& pat) const -> bool {
  if (_len == 0) {
    return false;
  }

  auto s = Pattern::into_searcher(pat, *this);
  return s.next()._type == SearchStep::Match;
}

auto Str::ends_with(auto&& pat) const -> bool {
  if (_len == 0) {
    return false;
  }

  auto s = Pattern::into_searcher(pat, *this);
  return s.next_back()._type == SearchStep::Match;
}

auto Str::trim_start_matches(auto&& pat) const -> Str {
  if (_len == 0) {
    return {};
  }

  auto s = Pattern::into_searcher(pat, *this);
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

  auto s = Pattern::into_searcher(pat, *this);
  auto j = usize{0U};
  if (auto m = s.next_reject_back()) {
    j = m._end;
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
  if (auto m = s.next_reject()) {
    i = m._pos;
    j = m._end;
  }
  if (auto m = s.next_reject_back()) {
    j = m._end;
  }
  return Str{_ptr + i, j - i};
}

auto Str::split_once(auto&& pat) const -> Option<Tuple<Str, Str>> {
  if (_len == 0) {
    return {};
  }

  auto s = Pattern::into_searcher(pat, *this);
  auto m = s.next_match();
  if (!m) {
    return {};
  }
  const auto i = m._pos;
  const auto j = m._end;
  return Tuple{Str{_ptr, i}, Str{_ptr + j, _len - j}};
}

auto Str::rsplit_once(auto&& pat) const -> Option<Tuple<Str, Str>> {
  if (_len == 0) {
    return {};
  }

  auto s = Pattern::into_searcher(pat, *this);
  auto m = s.next_match_back();
  if (!m) {
    return {};
  }
  const auto i = m._pos;
  const auto j = m._end;
  return Tuple{Str{_ptr, i}, Str{_ptr + j, _len - j}};
}

}  // namespace sfc::str

namespace sfc {
using str::Str;
}  // namespace sfc
