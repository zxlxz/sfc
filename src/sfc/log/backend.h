#pragma once

#include "mod.h"

namespace sfc::log {

struct IBackend {
  struct Meta {
    usize _size;
    void (*_dtor)(void*);
    void (Any::*_write_msg)(Level level, Str msg);

    template <class X>
    static auto of(const X&) -> Meta {
      return Meta{
          ._size = sizeof(X),
          ._dtor = [](void* p) { static_cast<X*>(p)->~X(); },
          ._write_msg = ops::fn(&X::write_msg),
      };
    }
  };

  trait::Any* _self = nullptr;
  const Meta* _meta = nullptr;

  IBackend(Any* self, const Meta& meta) : _self{self}, _meta{&meta} {}

 public:
  IBackend() = default;

  static auto from(auto& x) -> IBackend {
    static const auto meta = Meta::of(x);
    return IBackend{reinterpret_cast<Any*>(&x), meta};
  }

  void write_msg(Level level, Str msg) {
    const auto f = _meta->_write_msg;
    (_self->*f)(level, msg);
  }
};

}  // namespace sfc::log
