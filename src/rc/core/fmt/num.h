#pragma once

#include "rc/core/fmt/mod.h"
#include "rc/core/num/mod.h"

namespace rc::fmt {

template <class UInt>
pub auto int_to_str(UInt val, u8 type, Slice<u8> buf) -> Str;

pub auto flt_to_str(f64 val, u8 type, u32 prec, Slice<u8> buf) -> Str;

template <>
struct Display<u8> {
  const u8& _self;

  template <class Out>
  auto fmt(Formatter<Out>& f) const -> void {
    u8 buf[256];
    const auto s = fmt::int_to_str(_self, f.type(), buf);
    f.pad_integral(false, s);
  }
};

template <>
struct Display<u16> {
  const u16& _self;

  template <class Out>
  auto fmt(Formatter<Out>& f) const -> void {
    u8 buf[256];
    const auto s = fmt::int_to_str(_self, f.type(), buf);
    f.pad_integral(false, s);
  }
};

template <>
struct Display<u32> {
  const u32& _self;

  template <class Out>
  auto fmt(Formatter<Out>& f) const -> void {
    u8 buf[256];
    const auto s = fmt::int_to_str(_self, f.type(), buf);
    f.pad_integral(false, s);
  }
};

template <>
struct Display<u64> {
  const u64& _self;

  template <class Out>
  auto fmt(Formatter<Out>& f) const -> void {
    u8 buf[256];
    const auto s = fmt::int_to_str(_self, f.type(), buf);
    f.pad_integral(false, s);
  }
};

template <>
struct Display<i8> {
  const i8& _self;

  template <class Out>
  auto fmt(Formatter<Out>& f) const -> void {
    u8 buf[256];
    const auto s = fmt::int_to_str(num::abs(_self), f.type(), buf);
    f.pad_integral(_self < 0, s);
  }
};

template <>
struct Display<i16> {
  const i16& _self;

  template <class Out>
  auto fmt(Formatter<Out>& f) const -> void {
    u8 buf[256];
    const auto s = fmt::int_to_str(num::abs(_self), f.type(), buf);
    f.pad_integral(_self < 0, s);
  }
};

template <>
struct Display<i32> {
  const i32& _self;

  template <class Out>
  auto fmt(Formatter<Out>& f) const -> void {
    u8 buf[256];
    const auto s = fmt::int_to_str(num::abs(_self), f.type(), buf);
    f.pad_integral(_self < 0, s);
  }
};

template <>
struct Display<i64> {
  const i64& _self;

  template <class Out>
  auto fmt(Formatter<Out>& f) const -> void {
    u8 buf[256];
    const auto s = fmt::int_to_str(num::abs(_self), f.type(), buf);
    f.pad_integral(_self < 0, s);
  }
};

template <class T>
struct Display<T*> {
  T* _self;

  template <class Out>
  auto fmt(Formatter<Out>& f) const -> void {
    u8 buf[256];
    const auto type = f.type() == u8'x' ? u8'x' : u8'X';
    const auto s = fmt::int_to_str(usize(_self), type, buf);
    f.pad(s);
  }
};

template <>
struct Display<f32> {
  const f32& _self;

  template <class Out>
  auto fmt(Formatter<Out>& f) const -> void {
    u8 buf[256];
    const auto type = f.type();
    const auto prec = f.precision().unwrap_or(6);
    const auto s = fmt::flt_to_str(_self, type, prec, buf);
    f.pad(s);
  }
};

template <>
struct Display<f64> {
  const f64& _self;

  template <class Out>
  auto fmt(Formatter<Out>& f) const -> void {
    u8 buf[256];
    const auto type = f.type();
    const auto prec = f.precision().unwrap_or(6);
    const auto s = fmt::flt_to_str(_self, type, prec, buf);
    f.pad(s);
  }
};

}  // namespace rc::fmt
