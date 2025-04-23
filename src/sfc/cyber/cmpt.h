#pragma once

#include "sfc/sync.h"

namespace sfc::cyber {

class CmptBase {
  sync::Atomic<bool> _init{false};

 protected:
  CmptBase() = default;
  virtual ~CmptBase() = default;

  CmptBase(const CmptBase&) = delete;

 public:
  void init() {
    if (_init.load()) {
      return;
    }
    const auto ret = this->on_init();
    _init.store(ret);
  }

  void shutdown() {
    if (!_init.load()) {
      return;
    }
    this->on_shutdown();
  }

  virtual auto on_init() -> bool {
    return true;
  }

  virtual void on_shutdown() {}
};

}  // namespace sfc::cyber
