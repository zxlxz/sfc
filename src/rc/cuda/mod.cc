#include "rc.inl"

#include "rc/cuda/imp.h"

namespace rc::cuda {

#pragma region Device
pub auto Device::cnt() -> usize {
  // redirect: cuda::imp
  return imp::dev_cnt();
}

pub void Device::set(usize idx) {
  // redirect: cuda::imp
  return imp::dev_set(idx);
}

pub void Device::sync() {
  // redirect: cuda::imp
  return imp::dev_syn();
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
  imp::mem_set(p, 0, layout._size);
  return p;
}

pub auto Alloc::dealloc(void* p, Layout) -> void {
  if (p == nullptr) return;
  imp::mem_del(p);
}

pub void Alloc::copy(const void* src, void* dst, usize size) {
  if (size == 0) return;
  imp::mem_cpy(dst, src, size);
}

#pragma endregion

#pragma region Texture
pub TexImpl::TexImpl(u64 raw): _raw{raw} {}

pub TexImpl::~TexImpl() {
  if (_raw == u64(-1)) return;
}

pub TexImpl::TexImpl(TexImpl&& other) noexcept: _raw{other._raw} {
  other._raw = u64(-1);
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
