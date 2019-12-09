#include "rc.inl"

#include "rc/gpu/imp/cuda.h"
#include "rc/ffi.h"

#define CUDA_API_PER_THREAD_DEFAULT_STREAM
#include <cuda.h>
#pragma comment(lib, "cuda")

namespace rc::gpu::cuda {

#pragma region utils
using eid_t = ::cudaError_enum;

static auto eid2str(eid_t eid) -> Str {
  const char* val = nullptr;
  auto ret = ::cuGetErrorName(eid, &val);
  if (ret != eid_t(0)) return u8"Unknown";

  auto res = Str::from_cstr(val);
  return res;
}

template <class... U>
auto operator|(eid_t eid, fmt::Args<U...> args) -> void {
  if (eid == eid_t(0)) return;

  const auto name = cuda::eid2str(eid);
  rc::panic(u8"CUDA ERROR(`{}`): {}", name, args);
}

static void driver_init() {
  static auto eid = ::cuInit(0);
  eid | fmt::Args{u8"driver init failed"};
}
#pragma endregion

#pragma region dev
using dev_t = ::CUdevice;
using ctx_t = ::CUctx_st*;

auto _dev_cnt() -> usize {
  cuda::driver_init();

  static auto res = 0;
  static auto eid = ::cuDeviceGetCount(&res);
  eid | fmt::Args(u8"device get count failed");
  return res;
}

auto _dev_set(usize idx) -> void {
  const auto cnt = cuda::_dev_cnt();
  if (idx >= cnt) {
    rc::panic("invalid device , idx=`{}`, cnt=`{}`", idx, cnt);
  }

  dev_t dev = 0;
  ::cuDeviceGet(&dev, i32(idx)) | fmt::Args{u8"get device failed"};

  ctx_t ctx;
  ::cuDevicePrimaryCtxRetain(&ctx, dev) | fmt::Args{u8"get context failed"};
  ::cuCtxSetCurrent(ctx) | fmt::Args{u8"set context failed"};
}

auto _dev_syn() -> void {
  const auto eid = ::cuCtxSynchronize();
  eid | fmt::Args{u8"ctx sync failed"};
}
#pragma endregion

#pragma region stream
static thread_local auto _tls_stream = CU_STREAM_PER_THREAD;

auto _thr_new() -> thr_t {
  thr_t res;
  ::cuStreamCreate(&res, 0) | fmt::Args{u8"stream create failed"};
  return res;
}

auto _thr_del(thr_t thr) -> void {
  // 0: null
  // 1: CU_STREAM_LEGACY
  // 2: CU_STREAM_PER_THREAD
  if (u64(thr) <= 2) return;
  if (thr == _tls_stream) _tls_stream = CU_STREAM_PER_THREAD;

  const auto eid = ::cuStreamDestroy_v2(thr);
  eid | fmt::Args{u8"stream destroy failed"};
}

auto _thr_set(thr_t thr) -> void {
  if (u64(thr) <= 2) return;
  _tls_stream = thr;
}

auto _thr_syn(thr_t thr) -> void {
  const auto eid = ::cuStreamSynchronize_ptsz(thr);
  eid | fmt::Args{u8"stream sync failed"};
}

#pragma endregion

#pragma region impl
auto _mem_new(usize size, MemType type) -> void* {
  if (type == MemType::Device) {
    ::CUdeviceptr d = 0;
    const auto eid = ::cuMemAlloc_v2(&d, size);
    eid | fmt::Args{u8"mem alloc failed, size={}", size};
    return (void*)d;
  }
  if (type == MemType::Host) {
    void* h = nullptr;
    const auto eid = ::cuMemAllocHost_v2(&h, size);
    eid | fmt::Args{u8"mem alloc failed, size={}", size};
    return h;
  }
  return nullptr;
}

auto _mem_del(void* p, MemType type) -> void {
  if (type == MemType::Device) {
    const auto d = ::CUdeviceptr(p);
    const auto eid = ::cuMemFree_v2(d);
    eid | fmt::Args{u8"mem free failed, ptr={#x}", p};
  }
  if (type == MemType::Host) {
    const auto eid = ::cuMemFreeHost(p);
    eid | fmt::Args{u8"mem free failed, ptr={#x}", p};
  }
}

auto _mem_set(void* p, u8 val, usize size) -> void {
  const auto d = CUdeviceptr(p);
  const auto eid = ::cuMemsetD8Async_ptsz(d, val, size, _tls_stream);
  eid | fmt::Args{u8"memset failed, size={}", size};
}

auto _mem_cpy(void* dst, const void* src, usize size) -> void {
  const auto d = ::CUdeviceptr(dst);
  const auto s = ::CUdeviceptr(src);
  const auto eid = ::cuMemcpyAsync_ptsz(d, s, size, _tls_stream);
  eid | fmt::Args{u8"memcpy failed"};
}
#pragma endregion

#pragma region array
using arr_f = ::CUarray_format;

static auto _arr_fmt(ArrType type) -> arr_f {
  if (type._kind == ArrKind::UInt) {
    if (type._size == 8) return arr_f::CU_AD_FORMAT_UNSIGNED_INT8;
    if (type._size == 16) return arr_f::CU_AD_FORMAT_UNSIGNED_INT8;
    if (type._size == 32) return arr_f::CU_AD_FORMAT_UNSIGNED_INT8;
  }
  if (type._kind == ArrKind::SInt) {
    if (type._size == 8) return arr_f::CU_AD_FORMAT_SIGNED_INT8;
    if (type._size == 16) return arr_f::CU_AD_FORMAT_SIGNED_INT8;
    if (type._size == 32) return arr_f::CU_AD_FORMAT_SIGNED_INT8;
  }
  if (type._kind == ArrKind::Float) {
    if (type._size == 16) return arr_f::CU_AD_FORMAT_HALF;
    if (type._size == 32) return arr_f::CU_AD_FORMAT_FLOAT;
  }
  return arr_f(0);
}

static auto _fmt_size(arr_f f) -> usize {
  switch (f) {
    case CU_AD_FORMAT_UNSIGNED_INT8:
      return 1;
    case CU_AD_FORMAT_UNSIGNED_INT16:
      return 2;
    case CU_AD_FORMAT_UNSIGNED_INT32:
      return 4;
    case CU_AD_FORMAT_SIGNED_INT8:
      return 1;
    case CU_AD_FORMAT_SIGNED_INT16:
      return 2;
    case CU_AD_FORMAT_SIGNED_INT32:
      return 4;
    case CU_AD_FORMAT_HALF:
      return 2;
    case CU_AD_FORMAT_FLOAT:
      return 4;
  }
  return 0;
}

auto _arr_new(ArrType type, Extent dims, ArrFlag flags) -> arr_t {
  auto desc = ::CUDA_ARRAY3D_DESCRIPTOR{};
  desc.Width = dims._0;
  desc.Height = dims._1;
  desc.Depth = dims._2;
  desc.Flags = u32(flags);
  desc.NumChannels = type._channels;
  desc.Format = cuda::_arr_fmt(type);

  arr_t arr = nullptr;
  const auto eid = ::cuArray3DCreate_v2(&arr, &desc);
  eid | fmt::Args{u8"create array failed, dims={}", dims};
  return arr;
}

auto _arr_del(arr_t arr) -> void {
  if (arr == nullptr) return;
  const auto eid = ::cuArrayDestroy(arr);
  eid | fmt::Args{u8"destroy array failed"};
}

using arr_s = ::CUDA_ARRAY3D_DESCRIPTOR;
auto _arr_res(arr_t arr) -> arr_s {
  auto res = arr_s{};
  const auto eid = ::cuArray3DGetDescriptor_v2(&res, arr);
  eid | fmt::Args{u8"array get desc failed"};
  return res;
}

auto _arr_cpy(arr_t arr, const void* ptr, ArrCopy mode) -> void {
  const auto res = cuda::_arr_res(arr);
  const auto num_channels = cmp::max(res.NumChannels, 1u);
  const auto fmt_size = cuda::_fmt_size(res.Format);

  auto args = ::CUDA_MEMCPY3D{};
  args.WidthInBytes = fmt_size * num_channels * res.Width;
  args.Height = cmp::max(res.Height, usize(1));
  args.Depth = cmp::max(res.Depth, usize(1));

  if (mode == ArrCopy::P2A) {
    args.dstMemoryType = CUmemorytype::CU_MEMORYTYPE_ARRAY;
    args.dstArray = arr;

    args.srcMemoryType = CUmemorytype::CU_MEMORYTYPE_UNIFIED;
    args.srcDevice = ::CUdeviceptr(ptr);
    args.srcPitch = args.WidthInBytes;
    args.srcHeight = args.Height;
  } else {
    args.srcMemoryType = CUmemorytype::CU_MEMORYTYPE_ARRAY;
    args.srcArray = arr;

    args.dstMemoryType = CUmemorytype::CU_MEMORYTYPE_UNIFIED;
    args.dstDevice = ::CUdeviceptr(ptr);
    args.dstPitch = args.WidthInBytes;
    args.dstHeight = args.Height;
  }

  const auto eid = ::cuMemcpy3DAsync_v2_ptsz(&args, _tls_stream);
  eid | fmt::Args{u8"array copy failed"};
}
#pragma endregion

#pragma region texture
using res_f = CUresourceViewFormat;

static auto _res_fmt(const CUDA_ARRAY3D_DESCRIPTOR& desc) -> res_f {
  auto res = res_f::CU_RES_VIEW_FORMAT_NONE;

  switch (desc.Format) {
    case arr_f::CU_AD_FORMAT_UNSIGNED_INT8:
      res = res_f::CU_RES_VIEW_FORMAT_UINT_1X8;
      break;
    case arr_f::CU_AD_FORMAT_UNSIGNED_INT16:
      res = res_f::CU_RES_VIEW_FORMAT_UINT_1X16;
      break;
    case arr_f::CU_AD_FORMAT_UNSIGNED_INT32:
      res = res_f::CU_RES_VIEW_FORMAT_UINT_1X32;
      break;
    case arr_f::CU_AD_FORMAT_SIGNED_INT8:
      res = res_f::CU_RES_VIEW_FORMAT_SINT_1X8;
      break;
    case arr_f::CU_AD_FORMAT_SIGNED_INT16:
      res = res_f::CU_RES_VIEW_FORMAT_SINT_1X16;
      break;
    case arr_f::CU_AD_FORMAT_SIGNED_INT32:
      res = res_f::CU_RES_VIEW_FORMAT_SINT_1X8;
      break;
    case arr_f::CU_AD_FORMAT_HALF:
      res = res_f::CU_RES_VIEW_FORMAT_FLOAT_1X16;
      break;
    case arr_f::CU_AD_FORMAT_FLOAT:
      res = res_f::CU_RES_VIEW_FORMAT_FLOAT_1X32;
      break;
  }

  switch (desc.NumChannels) {
    case 1:
      break;
    case 2:
      res = res_f(u32(res) + 1);
      break;
    case 4:
      res = res_f(u32(res) + 2);
      break;
    default:
      break;
  }

  return res;
}

auto _tex_new(arr_t arr, TexAddr addr, TexFilter filter) -> tex_t {
  ::CUDA_ARRAY3D_DESCRIPTOR arr_desc = {};
  ::cuArray3DGetDescriptor_v2(&arr_desc, arr) |
      fmt::Args{u8"array get desc failed"};

  auto res_desc = ::CUDA_RESOURCE_DESC{};
  res_desc.resType = ::CU_RESOURCE_TYPE_ARRAY;
  res_desc.res.array.hArray = arr;

  auto tex_desc = ::CUDA_TEXTURE_DESC{};
  tex_desc.addressMode[0] = ::CUaddress_mode(addr);
  tex_desc.addressMode[1] = ::CUaddress_mode(addr);
  tex_desc.addressMode[2] = ::CUaddress_mode(addr);
  tex_desc.filterMode = ::CUfilter_mode(filter);

  CUDA_RESOURCE_VIEW_DESC res_view = {};
  res_view.width = arr_desc.Width;
  res_view.height = arr_desc.Height;
  res_view.depth = arr_desc.Depth;

  if (arr_desc.Flags == u32(ArrFlag::Layered)) {
    res_view.firstLayer = 0;
    res_view.lastLayer = u32(arr_desc.Depth - 1);
  }
  res_view.format = cuda::_res_fmt(arr_desc);

  unsigned long long tex;
  const auto eid = ::cuTexObjectCreate(&tex, &res_desc, &tex_desc, &res_view);
  eid | fmt::Args{u8"rc::cuda: texture create failed"};

  return tex_t(tex);
}

auto _tex_del(tex_t tex) -> void {
  if (tex == 0) return;
  const auto eid = ::cuTexObjectDestroy(tex);
  eid | fmt::Args{u8"destroy texture failed"};
}

auto _tex_arr(tex_t tex) -> arr_t {
  if (tex == tex_t(0)) return nullptr;
  auto res_desc = ::CUDA_RESOURCE_DESC{};

  const auto eid = ::cuTexObjectGetResourceDesc(&res_desc, u64(tex));
  eid | fmt::Args{u8"tex get desc failed"};
  return res_desc.res.array.hArray;
}

#pragma endregion

#pragma region mod
auto _mod_new(const void* dat) -> mod_t {
  mod_t mod = nullptr;
  const auto eid = ::cuModuleLoadData(&mod, dat);
  eid | fmt::Args{u8"module load data failed"};
  return mod;
}

auto _mod_del(mod_t mod) -> void {
  if (mod == nullptr) return;

  const auto eid = ::cuModuleUnload(mod);
  eid | fmt::Args{u8"module unload failed"};
}

auto _mod_fun(mod_t mod, Str name) -> fun_t {
  auto cname = ffi::CString{name};

  fun_t fun = nullptr;
  const auto eid = ::cuModuleGetFunction(&fun, mod, cname);
  eid | fmt::Args{u8"cannot find func `{}`", name};
  return fun;
}

auto _fun_run(fun_t f, Extent b, Extent t, const void* args[]) -> void {
  const auto bx = u32(b._0);
  const auto by = u32(b._1);
  const auto bz = u32(b._2);
  const auto tx = u32(t._0);
  const auto ty = u32(t._1);
  const auto tz = u32(t._2);

  const auto m = 0;                         // shared memory
  const auto s = _tls_stream;               // stream
  const auto v = const_cast<void**>(args);  // args
  const auto e = nullptr;                   // extra args

  const auto eid = ::cuLaunchKernel_ptsz(f, bx, by, bz, tx, ty, tz, m, s, v, e);
  eid | fmt::Args{u8"launch failed, blks={}, trds={}", b, t};
}
#pragma endregion

}  // namespace rc::gpu::cuda
