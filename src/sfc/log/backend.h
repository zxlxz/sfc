#pragma once

#include "sfc/alloc.h"

namespace sfc::log {

enum class Level {
  Trace,
  Debug,
  Info,
  Warn,
  Error,
  Fatal,
};

struct Record {
  Level level;
  Str time;
  Str msg;

 public:
  static auto tls_buf() -> String&;
  static auto time_str() -> Str;
};

class Backend {
  template <class T, class A>
  friend class boxed::Box;

  struct Meta {
    void (*_flush)(void*) = nullptr;
    void (*_write)(void*, Record) = nullptr;

    template <class X>
    static auto from(const X&) -> Meta {
      const auto flush = [](void* p) { static_cast<X*>(p)->flush(); };
      const auto write = [](void* p, Record r) { static_cast<X*>(p)->write(r); };
      return {flush, write};
    }
  };

  const Meta* _meta = nullptr;
  void* _self = nullptr;

 public:
  static auto from(auto& impl) -> Backend {
    static const auto meta = Meta::from(impl);
    auto res = Backend{};
    res._meta = &meta;
    res._self = &impl;
    return res;
  }

  explicit operator bool() const {
    return _self != nullptr;
  }

  void flush() {
    return (_meta->_flush)(_self);
  }

  void write(Record entry) {
    return (_meta->_write)(_self, entry);
  }
};

}  // namespace sfc::log
