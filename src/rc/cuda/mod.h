#pragma once

#include "rc/alloc.h"
#include "rc/math/mod.h"

struct CUstream_st;
struct CUarray_st;
struct CUmod_st;
struct CUfunc_st;

namespace rc::cuda {

using mod_t = ::CUmod_st*;
using thr_t = ::CUstream_st*;
using fun_t = ::CUfunc_st*;
using arr_t = ::CUarray_st*;
using tex_t = u64;
using dev_t = int;

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
  struct Arch {
    int _major;
    int _minor;

    template <class Out>
    void fmt(fmt::Formatter<Out>& formatter) const {
      formatter.write(u8"{}.{}", _major, _minor);
    }
  };

  dev_t _raw;

  pub static auto count() -> usize;
  pub static auto from_idx(usize idx) -> Device;
  pub static void sync();

  pub auto name() const -> String;
  pub auto arch() const -> Arch;
};
pub void set_device(const Device& idx);

struct Stream {
  thr_t _raw;

  pub explicit Stream(thr_t raw);
  pub ~Stream();
  pub Stream(Stream&& other) noexcept;

  pub static auto Default() -> Stream&;
  pub static auto with_flags(u32 flags) -> Stream;
  pub void sync();
};
pub void set_stream(const Stream&);

struct Memory {
  pub static void copy(const void* src, void* dst, usize size);
};

struct Alloc {
  using Layout = alloc::Layout;

  pub static auto alloc(Layout layout, MemType type) -> void*;
  pub static auto alloc_zeroed(Layout layout, MemType type) -> void*;
  pub static void dealloc(void* p, Layout layout);
};

struct TexImpl {
  using Dims = math::NDDims<3>;
  using Desc = cuda::TexDesc;
  tex_t _raw;

  pub TexImpl(tex_t raw);
  pub ~TexImpl();
  pub TexImpl(TexImpl&& other) noexcept;
  pub static auto with_dims(ArrXFmt xfmt, Dims dims, Desc desc) -> TexImpl;

  pub void set_data(const void* buf);
  pub void get_data(void* buf);
};

template <class T>
inline auto alloc(usize cnt, MemType type) -> T* {
  const auto layout = alloc::Layout::array<T>(cnt);
  const auto res = Alloc::alloc(layout, type);
  return ptr::cast<T>(res);
}

template <class T>
inline auto dealloc(T* p, usize cnt) -> void {
  const auto layout = alloc::Layout::array<T>(cnt);
  Alloc::dealloc(p, layout);
}

template <class T>
inline void copy(const T* src, T* dst, usize cnt) {
  const auto size = cnt * sizeof(T);
  Memory::copy(src, dst, size);
}

}  // namespace rc::cuda
