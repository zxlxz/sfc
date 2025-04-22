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
    void (*_dtor)(void*) = nullptr;
    void (*_flush)(void*) = nullptr;
    void (*_write_entry)(void*, Entry) = nullptr;
  };

  template <class X>
  static constexpr inline Meta META = {
      ._dtor = [](void* p) { delete static_cast<X*>(p); },
      ._flush = [](void* p) { return static_cast<X*>(p)->flush(); },
      ._write_entry = [](void* p, Entry entry) { return static_cast<X*>(p)->write_entry(entry); },
  };

  void*       _self = nullptr;
  const Meta* _meta = nullptr;

 public:
  void flush() {
    return (_meta->_flush)(_self);
  }

  void write_entry(Entry entry) {
    return (_meta->_write_entry)(_self, entry);
  }
};

}  // namespace sfc::log
