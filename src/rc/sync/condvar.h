#pragma once

#include "rc/sync/mutex.h"

namespace rc::sync {

using time::Duration;

struct CondVar {
  sys::sync::CondVar _inner;

  pub CondVar();
  pub ~CondVar();
  pub CondVar(CondVar&& other)noexcept;

  pub auto wait(MutexGuard& guard) -> void;
  pub auto wait_timeout(MutexGuard& guard, Duration dur) -> bool;
  pub auto notify_one() -> void;
  pub auto notify_all() -> void;
};

}  // namespace rc::sync
