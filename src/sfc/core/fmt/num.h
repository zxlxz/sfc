#pragma once

#include "sfc/core/fmt/fmter.h"

namespace sfc::fmt {

template <class T>
struct Int {
  T _val;

 public:
  auto to_str(const Style& style, slice::Slice<char> buf) const -> str::Str;

  void fmt(auto& f) const {
    char buf[64 + 16];

    const auto s = this->to_str(f.style(), buf);
    f.pad_num(_val < 0, s);
  }
};

template <class T>
struct Flt {
  T _val;

 public:
  auto to_str(const Style& style, slice::Slice<char> buf) const -> str::Str;

  void fmt(auto& f) const {
    char buf[64 + 16];

    const auto s = this->to_str(f.style(), buf);
    f.pad_num(_val < 0, s);
  }
};

struct Ptr {
  const void* _val;

 public:
  auto to_str(const Style& style, slice::Slice<char> buf) const -> str::Str;

  void fmt(auto& f) const {
    char buf[64 + 16];

    const auto s = this->to_str(f.style(), buf);
    f.pad_num(false, s);
  }
};

template <>
class IFmt<char> {
  char _val;

 public:
  void fmt(auto& f) const {
    f.pad(str::Str{&_val, 1});
  }
};

template <>
class IFmt<bool> {
  bool _val;

 public:
  void fmt(auto& f) const {
    f.pad(_val ? str::Str{"true"} : str::Str{"false"});
  }
};

template <trait::isInt T>
class IFmt<T> : Int<T> {
 public:
  using Int<T>::fmt;
};

template <trait::isFlt T>
class IFmt<T> : Flt<T> {
 public:
  using Flt<T>::fmt;
};

template <class T>
class IFmt<T*> : Ptr {
 public:
  using Ptr::fmt;
};

template <class T, usize N>
class IFmt<T[N]> {
  T _val[N];

 public:
  void fmt(auto& f) const {
    const auto v = slice::Slice<const T>{_val};
    f.debug_list().entries(v.iter());
  }
};

template <usize N>
class IFmt<char[N]> {
  char _val[N];

 public:
  void fmt(auto& f) const {
    f.pad(_val);
  }
};

template <>
class IFmt<const char*> {
  cstr_t _val;

 public:
  void fmt(auto& f) const {
    f.pad(str::Str::from_cstr(_val));
  }
};

template <>
class IFmt<char*> {
  cstr_t _val;

 public:
  void fmt(auto& f) const {
    f.pad(str::Str::from_cstr(_val));
  }
};

template <trait::isEnum T>
class IFmt<T> {
  T _val;

 public:
  void fmt(auto& f) const {
    const auto name = reflect::enum_name(_val);
    if (!name) {
      f.pad(name);
    } else {
      const auto int_val = static_cast<__underlying_type(T)>(_val);
      f.write(int_val);
    }
  }
};

}  // namespace sfc::fmt
