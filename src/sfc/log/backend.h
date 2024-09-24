#pragma once

#include "mod.h"

namespace sfc::log {

struct Entry {
  Level level;
  Str time;
  Str msg;
};

struct IBackend {
  struct Meta {
    usize _size;
    void (*_dtor)(void*);
    void (ops::Any::*_flush)();
    void (ops::Any::*_write_entry)(Entry);

    template <class X>
    static auto of(const X*) -> const Meta& {
      static const auto res = Meta{
          ._size = sizeof(X),
          ._dtor = [](void* p) { static_cast<X*>(p)->~X(); },
          ._flush = ops::fn(&X::flush),
          ._write_entry = ops::fn(&X::write_entry),
      };
      return res;
    }
  };

  void* _self = nullptr;
  const Meta* _meta = nullptr;

 public:
  IBackend() = default;

  explicit IBackend(auto* x) : _self{x}, _meta{&Meta::of(x)} {}

  void flush() {
    const auto p = static_cast<ops::Any*>(_self);
    const auto f = _meta->_flush;
    (p->*f)();
  }

  void write_entry(Entry entry) {
    const auto p = static_cast<ops::Any*>(_self);
    const auto f = _meta->_write_entry;
    (p->*f)(entry);
  }
};

}  // namespace sfc::log
