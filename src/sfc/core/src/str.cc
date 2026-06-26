#include "sfc/core/str.h"
#include "sfc/core/hash.h"

namespace sfc::str {

auto Str::trim_start() const noexcept -> Str {
  const auto is_space = [](char c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); };
  return this->trim_start_matches(is_space);
}

auto Str::trim_end() const noexcept -> Str {
  const auto is_space = [](char c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); };
  return this->trim_end_matches(is_space);
}

auto Str::trim() const noexcept -> Str {
  const auto is_space = [](char c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); };
  return this->trim_matches(is_space);
}

auto Str::hash() const noexcept -> usize {
  auto hasher = hash::Hasher{};
  hasher.write(this->as_bytes());
  return hasher.finish();
}

auto CharSearcher::next() -> SearchStep {
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

auto CharSearcher::next_back() -> SearchStep {
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

auto StrSearcher::match() const -> bool {
  if (_needle._len == 0) return true;
  if (_finger + _needle._len > _haystack._len) return false;

  const auto p = _haystack._ptr + _finger;
  return __builtin_memcmp(p, _needle._ptr, _needle._len) == 0;
}

auto StrSearcher::match_back() const -> bool {
  if (_needle._len == 0) return true;
  if (_finger_back < _needle._len) return false;

  const auto p = _haystack._ptr + _finger_back - _needle._len;
  return __builtin_memcmp(p, _needle._ptr, _needle._len) == 0;
}

auto StrSearcher::next() -> SearchStep {
  if (_finger >= _haystack._len) {
    return {SearchStep::Done, 0, 0};
  }

  const auto old_finger = _finger;
  if (this->match()) {
    _finger += _needle._len;
    return {SearchStep::Match, old_finger, _finger};
  } else {
    if (_finger + _needle._len < _haystack._len) {
      _finger += 1;
    } else {
      _finger = _haystack._len;
    }
    return {SearchStep::Reject, old_finger, _finger};
  }
}

auto StrSearcher::next_back() -> SearchStep {
  if (_finger_back == 0) {
    return {SearchStep::Done, 0, 0};
  }

  const auto old_finger_back = _finger_back;
  if (this->match_back()) {
    _finger_back -= _needle._len;
    return {SearchStep::Match, _finger_back, old_finger_back};
  } else {
    if (_finger_back >= _needle._len) {
      _finger_back -= _needle._len;
    } else {
      _finger_back = 0;
    }
    return {SearchStep::Reject, _finger_back, old_finger_back};
  }
}

}  // namespace sfc::str
