#pragma once

#include "mod.h"

namespace sfc::log {

struct IBackend {
  struct Meta {
    usize _size;
    void (*_dtor)(void*);
    void (trait::Any::*_write_msg)(Level level, Str msg);

    template <class X>
    static auto of(const X*) -> const Meta& {
      static const auto res = Meta{
          ._size = sizeof(X),
          ._dtor = [](void* p) { static_cast<X*>(p)->~X(); },
          ._write_msg = ops::fn(&X::write_msg),
      };
      return res;
    }
  };

  void* _self = nullptr;
  const Meta* _meta = nullptr;

 public:
  IBackend() = default;

  explicit IBackend(auto* x) : _self{x}, _meta{&Meta::of(x)} {}

  void write_msg(Level level, Str msg) {
    const auto p = static_cast<trait::Any*>(_self);
    const auto f = _meta->_write_msg;
    (p->*f)(level, msg);
  }
};

}  // namespace sfc::log
