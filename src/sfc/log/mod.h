#pragma once

#include "sfc/collections.h"

namespace sfc::log {

enum class Level {
  Trace,
  Debug,
  Info,
  Warning,
  Error,
  Fatal,
};
SFC_ENUM(Level, Trace, Debug, Info, Warning, Error, Fatal)

struct IBackend {
  struct Meta {
    usize _size;
    void (*_dtor)(void*);
    void (Any::*_write_msg)(Level level, Str msg);

    template <class X>
    static auto of(const X&) -> const Meta& {
      static const auto res = Meta{
          ._size = sizeof(X),
          ._dtor = [](void* p) { static_cast<X*>(p)->~X(); },
          ._write_msg = Any::cast(&X::write_msg),
      };
      return res;
    }
  };

  Any* _self;
  const Meta* _meta;

 public:
  template <class X>
  explicit IBackend(X& x) : _self{reinterpret_cast<Any*>(&x)}, _meta{&Meta::of(x)} {}

  void write_msg(Level level, Str msg) {
    (_self->*_meta->_write_msg)(level, msg);
  }
};

}  // namespace sfc::log
