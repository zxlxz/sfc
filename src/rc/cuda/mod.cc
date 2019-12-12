#include "rc.inl"

#include "rc/cuda/imp.h"

namespace rc::cuda {

#pragma region Device
pub auto Device::count() -> usize {
  // redirect -> imp
  return imp::dev_cnt();
}

pub void Device::sync() {
  // redirect -> imp
  return imp::dev_sync();
}

pub auto Device::from_idx(usize idx) -> Device {
  const auto raw = imp::dev_raw(idx);
  return Device{raw};
}

pub auto Device::name() const -> String {
  // redirect -> imp
  return imp::dev_name(_raw);
}

pub auto Device::arch() const -> Device::Arch {
  // redirect -> imp
  return imp::dev_arch(_raw);
}

pub void set_device(const Device& dev) {
  const auto ctx = imp::dev_ctx(dev._raw);
  imp::set_ctx(ctx);
}
#pragma endregion

#pragma region Stream
pub Stream::Stream(thr_t raw) : _raw{raw} {}

pub Stream::~Stream() {
  if (_raw == nullptr) return;
  imp::stream_del(_raw);
}

pub Stream::Stream(Stream&& other) noexcept : _raw{other._raw} {
  _raw = nullptr;
}

pub auto Stream::Default() -> Stream& {
  static auto thr = imp::stream_default();
  static auto res = Stream{thr};
  return res;
}

pub auto Stream::with_flags(u32 flags) -> Stream {
  const auto thr = imp::stream_new(flags);
  return Stream{thr};
}

pub void Stream::sync() {
  // redirect: imp
  imp::stream_sync(_raw);
}

pub void set_stream(const Stream& stream) {
  imp::set_stream(stream._raw);
}
#pragma endregion

#pragma region Memory
pub void Memory::copy(const void* src, void* dst, usize size) {
  if (size == 0) return;
  imp::mem_copy(dst, src, size);
}
#pragma endregion

#pragma region Alloc
pub auto Alloc::alloc(Layout layout, MemType type) -> void* {
  if (layout._size == 0) return nullptr;
  const auto p = imp::mem_new(layout._size, type);
  return p;
}

pub auto Alloc::alloc_zeroed(Layout layout, MemType type) -> void* {
  if (layout._size == 0) return nullptr;
  const auto p = imp::mem_new(layout._size, type);
  imp::mem_set(p, u8(0), layout._size);
  return p;
}

pub void Alloc::dealloc(void* p, Layout) {
  if (p == nullptr) return;
  imp::mem_del(p);
}
#pragma endregion

#pragma region Texture
pub TexImpl::TexImpl(tex_t raw): _raw{raw} {}

pub TexImpl::~TexImpl() {
  if (_raw == tex_t(-1)) return;
}

pub TexImpl::TexImpl(TexImpl&& other) noexcept: _raw{other._raw} {
  other._raw = tex_t(-1);
}

pub auto TexImpl::with_dims(ArrXFmt xfmt, Dims dims, Desc desc) -> TexImpl {
  const auto arr = imp::arr_new(xfmt, dims, ArrFlag::Default);
  const auto tex = imp::tex_new(arr, desc);
  return TexImpl{tex};
}

pub void TexImpl::set_data(const void* buf) {
  const auto arr = imp::tex_arr(_raw);
  imp::arr_set(arr, buf);
}

pub void TexImpl::get_data(void* buf) {
  const auto arr = imp::tex_arr(_raw);
  imp::arr_get(arr, buf);
}
#pragma endregion

}  // namespace rc::cuda
