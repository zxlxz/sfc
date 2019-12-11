#pragma once

#include "rc/alloc.h"
#include "rc/math/mod.h"

namespace rc::cuda {

enum class MemType {
  Host = 0x1,
  Device = 0x2,
};

enum class ArrFlag {
  Default = 0x0,
  Layered = 0x1,
  Surface = 0x2,
  Cubemap = 0x4,
  TexGather = 0x8
};

enum class ArrFmt {
  U8 = 0x01,
  U16 = 0x02,
  U32 = 0x03,
  I8 = 0x08,
  I16 = 0x09,
  I32 = 0x0a,
  F16 = 0x10,
  F32 = 0x20,
};

struct ArrXFmt {
  ArrFmt _fmt;
  u32 _channels;
};

enum class TexAddr {
  Wrap = 0,
  Clamp = 1,
  Mirror = 2,
  Border = 3,
};

enum class TexFilter {
  Point = 0,
  Liner = 1,
};

struct TexDesc {
  TexAddr _addr;
  TexFilter _filter;
};

struct FnDims {
  math::NDDims<3> _blk;
  math::NDDims<3> _trd;
};

#pragma region IntoArrXFmt
template <class T>
struct IntoArrXFmt;

template <>
struct IntoArrXFmt<u8> {
  static auto into() -> ArrXFmt { return ArrXFmt{ArrFmt::U8, 1}; }
};

template <>
struct IntoArrXFmt<u16> {
  static auto into() -> ArrXFmt { return ArrXFmt{ArrFmt::U16, 1}; }
};

template <>
struct IntoArrXFmt<u32> {
  static auto into() -> ArrXFmt { return ArrXFmt{ArrFmt::U32, 1}; }
};

template <>
struct IntoArrXFmt<i8> {
  static auto into() -> ArrXFmt { return ArrXFmt{ArrFmt::I8, 1}; }
};

template <>
struct IntoArrXFmt<i16> {
  static auto into() -> ArrXFmt { return ArrXFmt{ArrFmt::I16, 1}; }
};

template <>
struct IntoArrXFmt<i32> {
  static auto into() -> ArrXFmt { return ArrXFmt{ArrFmt::I32, 1}; }
};

template <>
struct IntoArrXFmt<f32> {
  static auto into() -> ArrXFmt { return ArrXFmt{ArrFmt::F32, 1}; }
};
#pragma endregion

struct Device {
  pub auto cnt() -> usize;
  pub void set(usize idx);
  pub void sync();
};

struct Alloc {
  using Layout = alloc::Layout;

  pub static auto alloc(Layout layout, MemType type) -> void*;
  pub static auto alloc_zeroed(Layout layout, MemType type) -> void*;
  pub static void dealloc(void* p, Layout layout);
  pub static void copy(const void* src, void* dst, usize size);
};

struct TexImpl {
  using Dims = math::NDDims<3>;
  using Desc = cuda::TexDesc;
  u64 _raw;

  pub TexImpl(u64 raw);
  pub ~TexImpl();
  pub TexImpl(TexImpl&& other) noexcept;
  pub static auto with_dims(ArrXFmt xfmt, Dims dims, Desc desc) -> TexImpl;

  pub void set_data(const void* buf);
  pub void get_data(void* buf);
};

template <class T>
void copy(const T* src, T* dst, usize cnt) {
  Alloc::copy(src, dst, cnt * sizeof(T));
}

}  // namespace rc::cuda
