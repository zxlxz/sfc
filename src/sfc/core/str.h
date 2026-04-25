#pragma once

#include "sfc/core/convert.h"
#include "sfc/core/hash.h"
#include "sfc/core/num.h"
#include "sfc/core/slice.h"

namespace sfc::str {

struct Chars;

struct Str {
  const char* _ptr = nullptr;
  usize _len = 0;

 public:
  [[gnu::always_inline]] constexpr Str() noexcept = default;

  [[gnu::always_inline]] constexpr Str(const char* s, usize n) noexcept : _ptr{s}, _len{n} {}

  [[gnu::always_inline]] constexpr Str(const char* s) noexcept
      : _ptr{s}, _len{s ? __builtin_strlen(s) : 0} {}

  [[gnu::always_inline]] static auto from_utf8(Slice<const u8> s) noexcept -> Str {
    return {reinterpret_cast<const char*>(s._ptr), s._len};
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
    return {reinterpret_cast<const u8*>(_ptr), _len};
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
    return Iter{{}, _ptr, _ptr + _len};
  }

  auto chars() const noexcept -> Chars;

 public:
  auto find(auto&& pat) const -> Option<usize>;
  auto rfind(auto&& pat) const -> Option<usize>;

  auto contains(auto&& pat) const -> bool;
  auto starts_with(auto&& pat) const -> bool;
  auto ends_with(auto&& pat) const -> bool;

  auto trim_start_matches(auto&& pat) const -> Str;
  auto trim_end_matches(auto&& pat) const -> Str;
  auto trim_matches(auto&& pat) const -> Str;

  auto trim_start() const noexcept -> Str {
    const auto is_space = [](char c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); };
    return this->trim_start_matches(is_space);
  }

  auto trim_end() const noexcept -> Str {
    const auto is_space = [](char c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); };
    return this->trim_end_matches(is_space);
  }

  auto trim() const noexcept -> Str {
    const auto is_space = [](char c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); };
    return this->trim_matches(is_space);
  }

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
    if (auto t = f._spec.type(); t == '?' || t == 's') {
      f.write_char('"');
      f.pad(*this);
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
  auto hash() const noexcept -> usize {
    auto imp = hash::Hasher{};
    for (auto i = 0UL; i < _len; ++i) {
      imp.write_byte(_ptr[i]);
    }
    return imp.finish();
  }
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

struct Chars : iter::Iterator<char32_t> {
  const char* _ptr;
  const char* _end;

 public:
  auto next() noexcept -> Option<char32_t>;
  auto next_back() noexcept -> Option<char32_t>;
};

inline auto Str::chars() const noexcept -> Chars {
  return {{}, _ptr, _ptr + _len};
}

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
  F _pred;
  u32 _finger = 0;
  u32 _finger_back = static_cast<u32>(_haystack._len);

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
      return CharPredicateSearcher{{haystack}, mem::move(self)};
    }
  }
};

auto Str::find(auto&& pat) const -> Option<usize> {
  if (_len == 0) {
    return {};
  }

  auto s = Pattern::into_searcher(pat, *this);
  return s.next_match().pos();
}

auto Str::rfind(auto&& pat) const -> Option<usize> {
  if (_len == 0) {
    return {};
  }

  auto s = Pattern::into_searcher(pat, *this);
  return s.next_match_back().pos();
}

auto Str::contains(auto&& pat) const -> bool {
  if (_len == 0) {
    return false;
  }

  auto s = Pattern::into_searcher(pat, *this);
  return s.next_match()._type == SearchStep::Match;
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

  auto s = Pattern::into_searcher(pat, *this);
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

namespace sfc {
using str::Str;
}  // namespace sfc
