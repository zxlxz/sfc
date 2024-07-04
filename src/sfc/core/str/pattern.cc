#include "pattern.h"

namespace sfc::str::pattern {

auto StrSearcher::next() -> SearchStep {
  if (_finger >= _finger_back) {
    return {SearchStep::Done};
  }

  const auto ss = _haystack.get_unchecked({_finger, _finger + _needle._len});
  const auto mm = ss == _needle ? SearchStep::Match : SearchStep::Reject;
  const auto res = SearchStep{mm, {_finger, _finger + _needle._len}};
  _finger += mm == SearchStep::Match ? _needle._len : 1U;
  return res;
}

auto StrSearcher::next_back() -> SearchStep {
  if (_finger >= _finger_back) {
    return {SearchStep::Done};
  }

  const auto ch = _haystack.get_unchecked({_finger_back - _needle._len, _finger_back});
  const auto mm = ch == _needle ? SearchStep::Match : SearchStep::Reject;
  const auto res = SearchStep{mm, {_finger_back, _finger_back + 1}};
  _finger_back -= mm == SearchStep::Match ? _needle._len : 1U;
  return res;
}

}  // namespace sfc::str::pattern
