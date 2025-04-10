#pragma once

#include "mod.h"

namespace sfc::log {

struct Entry {
  Level level;
  Str   time;
  Str   msg;
};

class IBackend {
  friend class Box<IBackend&>;

  struct Meta {
    void (*_drop)(void*) = nullptr;
    void (IBackend::*_flush)() = nullptr;
    void (IBackend::*_write_entry)(Entry) = nullptr;
  };

  template <class X>
  static const inline Meta META = ops::dyn_meta<IBackend, Meta>(&X::flush, &X::write_entry);

  IBackend*   _self = nullptr;
  const Meta* _meta = nullptr;

 public:
  IBackend() = default;

  template <class X>
  explicit IBackend(X& x) noexcept : _self{reinterpret_cast<IBackend*>(&x)}, _meta{&META<X>} {}

  void flush() {
    return (_self->*_meta->_flush)();
  }

  void write_entry(Entry entry) {
    return (_self->*_meta->_write_entry)(entry);
  }
};

}  // namespace sfc::log
