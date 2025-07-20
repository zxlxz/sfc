#pragma once

#include "sfc/core/num.h"
#include "sfc/core/reflect.h"

namespace sfc::fmt {

struct Style;

template <class T>
struct IFmt;

template <class T>
IFmt(const T&) -> IFmt<T>;

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
  void fmt(auto& f) const {
    char buf[8 * sizeof(_val)];

    const auto digits = num::int2str(buf, num::abs(_val), f.style()._type);
    f.pad_num(_val < 0, digits);
  }
};

template <>
struct IFmt<short> {
  short _val;

 public:
  void fmt(auto& f) const {
    char buf[8 * sizeof(_val)];

    const auto digits = num::int2str(buf, num::abs(_val), f.style()._type);
    f.pad_num(_val < 0, digits);
  }
};

template <>
struct IFmt<int> {
  int _val;

 public:
  void fmt(auto& f) const {
    char buf[8 * sizeof(_val)];

    const auto digits = num::int2str(buf, num::abs(_val), f.style()._type);
    f.pad_num(_val < 0, digits);
  }
};

template <>
struct IFmt<long> {
  long _val;

 public:
  void fmt(auto& f) const {
    char buf[8 * sizeof(_val)];

    const auto digits = num::int2str(buf, num::abs(_val), f.style()._type);
    f.pad_num(_val < 0, digits);
  }
};

template <>
struct IFmt<long long> {
  long long _val;

 public:
  void fmt(auto& f) const {
    char buf[8 * sizeof(_val)];

    const auto digits = num::int2str(buf, num::abs(_val), f.style()._type);
    f.pad_num(_val < 0, digits);
  }
};

template <>
struct IFmt<unsigned char> {
  unsigned char _val;

 public:
  void fmt(auto& f) const {
    char buf[8 * sizeof(_val)];

    const auto digits = num::int2str(buf, _val, f.style()._type);
    f.pad_num(false, digits);
  }
};

template <>
struct IFmt<unsigned short> {
  unsigned short _val;

 public:
  void fmt(auto& f) const {
    char buf[8 * sizeof(_val)];

    const auto digits = num::int2str(buf, _val, f.style()._type);
    f.pad_num(false, digits);
  }
};

template <>
struct IFmt<unsigned int> {
  unsigned int _val;

 public:
  void fmt(auto& f) const {
    char buf[8 * sizeof(_val)];

    const auto s = num::int2str(buf, _val, f.style()._type);
    f.pad_num(false, s);
  }
};

template <>
struct IFmt<unsigned long> {
  unsigned long _val;

 public:
  void fmt(auto& f) const {
    char buf[8 * sizeof(_val)];

    const auto s = num::int2str(buf, _val, f.style()._type);
    f.pad_num(false, s);
  }
};

template <>
struct IFmt<unsigned long long> {
  unsigned long long _val;

 public:
  void fmt(auto& f) const {
    char buf[8 * sizeof(_val)];

    const auto s = num::int2str(buf, _val, f.style()._type);
    f.pad_num(false, s);
  }
};

template <>
struct IFmt<float> {
  float _val;

 public:
  void fmt(auto& f) const {
    char buf[64];

    const auto s = num::flt2str(buf, num::fabs(_val), f.style().precision(4), f.style()._type);
    f.pad_num(_val < 0, s);
  }
};

template <>
struct IFmt<double> {
  double _val;

 public:
  void fmt(auto& f) const {
    char buf[64];

    const auto s = num::flt2str(buf, num::fabs(_val), f.style().precision(6), f.style()._type);
    f.pad_num(_val < 0, s);
  }
};

template <class T, usize N>
struct IFmt<T[N]> {
  const T (&_val)[N];

 public:
  void fmt(auto& f) const {
    const auto v = slice::Slice<const T>{_val};
    f.debug_list().entries(v.iter());
  }
};

template <usize N>
struct IFmt<char[N]> {
  const char (&_val)[N];

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

}  // namespace sfc::fmt
