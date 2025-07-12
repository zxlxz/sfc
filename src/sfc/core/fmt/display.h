#pragma once

#include "sfc/core/reflect.h"

namespace sfc::fmt {

struct Style;

template <class T>
struct IFmt {
  T _val;

 public:
  void fmt(auto& f) const {
    const auto info = reflect_struct(_val);

    auto imp = f.debug_struct();
    info.fields().map([&](const auto& item) { imp.field(item.name, item.value); });
  }
};

template <>
struct IFmt<bool> {
  bool _val;

 public:
  void fmt(auto& f) const {
    f.pad(_val ? str::Str{"true"} : str::Str{"false"});
  }
};

template <>
struct IFmt<char> {
  char _val;

 public:
  void fmt(auto& f) const {
    f.pad(str::Str{&_val, 1});
  }
};

template <>
struct IFmt<signed char> {
  signed char _val;

 public:
  auto to_str(const Style& style, slice::Slice<char> buf) const -> str::Str;

  void fmt(auto& f) const {
    char buf[8 * sizeof(_val) + 8];

    const auto s = this->to_str(f.style(), buf);
    f.pad_num(_val < 0, s);
  }
};

template <>
struct IFmt<short> {
  short _val;

 public:
  auto to_str(const Style& style, slice::Slice<char> buf) const -> str::Str;

  void fmt(auto& f) const {
    char buf[8 * sizeof(_val) + 8];

    const auto s = this->to_str(f.style(), buf);
    f.pad_num(_val < 0, s);
  }
};

template <>
struct IFmt<int> {
  int _val;

 public:
  auto to_str(const Style& style, slice::Slice<char> buf) const -> str::Str;

  void fmt(auto& f) const {
    char buf[8 * sizeof(_val) + 8];

    const auto s = this->to_str(f.style(), buf);
    f.pad_num(_val < 0, s);
  }
};

template <>
struct IFmt<long> {
  long _val;

 public:
  auto to_str(const Style& style, slice::Slice<char> buf) const -> str::Str;

  void fmt(auto& f) const {
    char buf[8 * sizeof(_val) + 8];

    const auto s = this->to_str(f.style(), buf);
    f.pad_num(_val < 0, s);
  }
};

template <>
struct IFmt<long long> {
  long long _val;

 public:
  auto to_str(const Style& style, slice::Slice<char> buf) const -> str::Str;

  void fmt(auto& f) const {
    char buf[8 * sizeof(_val) + 8];

    const auto s = this->to_str(f.style(), buf);
    f.pad_num(_val < 0, s);
  }
};

template <>
struct IFmt<unsigned char> {
  unsigned char _val;

 public:
  auto to_str(const Style& style, slice::Slice<char> buf) const -> str::Str;

  void fmt(auto& f) const {
    char buf[8 * sizeof(_val) + 8];

    const auto s = this->to_str(f.style(), buf);
    f.pad_num(false, s);
  }
};

template <>
struct IFmt<unsigned short> {
  unsigned short _val;

 public:
  auto to_str(const Style& style, slice::Slice<char> buf) const -> str::Str;

  void fmt(auto& f) const {
    char buf[8 * sizeof(_val) + 8];

    const auto s = this->to_str(f.style(), buf);
    f.pad_num(false, s);
  }
};

template <>
struct IFmt<unsigned int> {
  unsigned int _val;

 public:
  auto to_str(const Style& style, slice::Slice<char> buf) const -> str::Str;

  void fmt(auto& f) const {
    char buf[8 * sizeof(_val) + 8];

    const auto s = this->to_str(f.style(), buf);
    f.pad_num(false, s);
  }
};

template <>
struct IFmt<unsigned long> {
  unsigned long _val;

 public:
  auto to_str(const Style& style, slice::Slice<char> buf) const -> str::Str;

  void fmt(auto& f) const {
    char buf[8 * sizeof(_val) + 8];

    const auto s = this->to_str(f.style(), buf);
    f.pad_num(false, s);
  }
};

template <>
struct IFmt<unsigned long long> {
  unsigned long long _val;

 public:
  auto to_str(const Style& style, slice::Slice<char> buf) const -> str::Str;

  void fmt(auto& f) const {
    char buf[8 * sizeof(unsigned long long) + 8];

    const auto s = this->to_str(f.style(), buf);
    f.pad_num(false, s);
  }
};

template <>
struct IFmt<float> {
  float _val;

 public:
  auto to_str(const Style& style, slice::Slice<char> buf) const -> str::Str;

  void fmt(auto& f) const {
    char buf[8 * sizeof(_val) + 8];

    const auto s = this->to_str(f.style(), buf);
    f.pad_num(_val < 0, s);
  }
};

template <>
struct IFmt<double> {
  double _val;

 public:
  auto to_str(const Style& style, slice::Slice<char> buf) const -> str::Str;

  void fmt(auto& f) const {
    char buf[8 * sizeof(_val) + 8];

    const auto s = this->to_str(f.style(), buf);
    f.pad_num(_val < 0, s);
  }
};

template <>
struct IFmt<const void*> {
  const void* _val;

 public:
  auto to_str(const Style& style, slice::Slice<char> buf) const -> str::Str;

  void fmt(auto& f) const {
    char buf[8 * sizeof(_val) + 8];

    const auto s = this->to_str(f.style(), buf);
    f.pad_num(false, s);
  }
};

template <class T>
struct IFmt<T*> : IFmt<const void*> {
  using IFmt<const void*>::IFmt;
};

template <class T>
struct IFmt<const T*> : IFmt<T*> {
  using IFmt<T*>::IFmt;
};

template <class T, usize N>
struct IFmt<T[N]> {
  T _val[N];

 public:
  void fmt(auto& f) const {
    const auto v = slice::Slice<const T>{_val};
    f.debug_list().entries(v.iter());
  }
};

template <usize N>
struct IFmt<char[N]> {
  char _val[N];

 public:
  void fmt(auto& f) const {
    f.pad(_val);
  }
};

template <>
struct IFmt<const char*> {
  const char* _val;

 public:
  void fmt(auto& f) const {
    f.pad(str::Str::from_cstr(_val));
  }
};

template <>
struct IFmt<char*> {
  const char* _val;

 public:
  void fmt(auto& f) const {
    f.pad(str::Str::from_cstr(_val));
  }
};

template <class T>
  requires(__is_enum(T))
struct IFmt<T> {
  T _val;

 public:
  void fmt(auto& f) const {
    using U = __underlying_type(T);

    const auto name = reflect::enum_name(_val);
    if (!name) {
      IFmt<U>{static_cast<U>(_val)}.fmt(f);
      return;
    }

    f.pad(name);
  }
};

}  // namespace sfc::fmt
