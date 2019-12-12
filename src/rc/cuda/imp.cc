#include "rc/cuda/imp.h"

#include "rc.inl"
#include "rc/ffi.h"

#define CUDA_API_PER_THREAD_DEFAULT_STREAM
#include <cuda.h>
#pragma comment(lib, "cuda")

#pragma warning(disable: 26812)

namespace rc::cuda::imp {

#pragma region utils
using eid_t = ::CUresult;

static auto err2str(eid_t eid) -> Str {
  const char* val = nullptr;
  auto ret = ::cuGetErrorName(eid, &val);
  if (ret != eid_t(0)) return u8"Unknown";

  auto res = Str::from_cstr(val);
  return res;
}

template <class... U>
auto operator|(eid_t eid, fmt::Args<U...> args) -> void {
  if (eid == eid_t(0)) return;

  const auto name = imp::err2str(eid);
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

auto dev_cnt() -> usize {
  imp::driver_init();

  static auto res = 0;
  static auto eid = ::cuDeviceGetCount(&res);
  eid | fmt::Args(u8"device get count failed");
  return res;
}

auto dev_raw(usize idx) -> dev_t {
  const auto cnt = imp::dev_cnt();
  if (idx >= cnt) {
    rc::panic("invalid device , idx=`{}`, cnt=`{}`", idx, cnt);
  }

  dev_t dev = 0;
  ::cuDeviceGet(&dev, i32(idx)) | fmt::Args{u8"get device failed"};
  return dev;
}

auto dev_ctx(dev_t dev) -> ctx_t {
  ctx_t ctx;
  ::cuDevicePrimaryCtxRetain(&ctx, dev) | fmt::Args{u8"get context failed"};
  return ctx;
}

auto set_ctx(ctx_t ctx) -> void {
  ::cuCtxSetCurrent(ctx) | fmt::Args{u8"set context failed"};
}

auto dev_name(dev_t dev) -> String {
  char buf[256];
  ::cuDeviceGetName(buf, sizeof(buf), dev) |
      fmt::Args{u8"get device name failed"};
  const auto res = Str::from_cstr(buf);
  return String::from(res);
}

auto dev_arch(dev_t dev) -> Device::Arch {
  const auto GET_MAJOR = CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR;
  const auto GET_MINOR = CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR;

  auto res = Device::Arch{0, 0} ;
  (void)::cuDeviceGetAttribute(&res._major, GET_MAJOR, dev);
  (void)::cuDeviceGetAttribute(&res._minor, GET_MINOR, dev);
  return res;
}

auto dev_sync() -> void {
  const auto eid = ::cuCtxSynchronize();
  eid | fmt::Args{u8"ctx sync failed"};
}

#pragma endregion

#pragma region stream
static thread_local auto _tls_stream = CU_STREAM_PER_THREAD;

auto stream_new(u32 flags) -> thr_t {
  thr_t res;
  ::cuStreamCreate(&res, flags) | fmt::Args{u8"stream create failed"};
  return res;
}

auto stream_del(thr_t thr) -> void {
  // 0: null
  // 1: CU_STREAM_LEGACY
  // 2: CU_STREAM_PER_THREAD
  if (u64(thr) <= 0x2) {
    return;
  }

  const auto eid = ::cuStreamDestroy_v2(thr);
  eid | fmt::Args{u8"stream destroy failed"};

  if (thr == _tls_stream) {
    _tls_stream = imp::stream_default();
  }
}

auto stream_default() -> thr_t {
  const auto ret = CU_STREAM_PER_THREAD;
  return ret;
}

auto set_stream(thr_t thr) -> void {
  if (u64(thr) <= 2) return;
  _tls_stream = thr;
}

auto stream_sync(thr_t thr) -> void {
  const auto eid = ::cuStreamSynchronize_ptsz(thr);
  eid | fmt::Args{u8"stream sync failed"};
}

#pragma endregion

#pragma region impl

auto mem_type(const void* p) -> MemType {
  auto res = CUmemorytype_enum::CU_MEMORYTYPE_UNIFIED;

  const auto eid = ::cuPointerGetAttribute(
      ptr::cast<int>(&res), CU_POINTER_ATTRIBUTE_MEMORY_TYPE, ::CUdeviceptr(p));

  eid | fmt::Args{u8"get memory type failed"};

  switch (res) {
    case CU_MEMORYTYPE_DEVICE:
      return MemType::Device;
    case CU_MEMORYTYPE_HOST:
      return MemType::Host;
    default:
      rc::panic(u8"CUDA ERROR: unknow memory type");
      return MemType::Host;
  }
}

auto mem_new(usize size, MemType type) -> void* {
  void* p = nullptr;
  const auto eid = [&]() {
    switch (type) {
      case MemType::Device:
        return ::cuMemAlloc_v2(ptr::cast<::CUdeviceptr>(&p), size);
      case MemType::Host:
        return ::cuMemAllocHost_v2(&p, size);
      default:
        return CUDA_ERROR_INVALID_VALUE;
    }
  }();
  eid | fmt::Args{u8"mem alloc failed, size={}", size};
  return p;
}

auto mem_del(void* p) -> void {
  const auto type = imp::mem_type(p);
  const auto eid = [=]() {
    switch (type) {
      case MemType::Device:
        return ::cuMemFree_v2(::CUdeviceptr(p));
      case MemType::Host:
        return ::cuMemFreeHost(p);
      default:
        return CUDA_ERROR_INVALID_VALUE;
    }
  }();
  eid | fmt::Args{u8"mem free failed, ptr={#x}", p};
}

auto mem_set(void* p, u8 val, usize size) -> void {
  const auto d = CUdeviceptr(p);
  const auto eid = ::cuMemsetD8Async_ptsz(d, val, size, _tls_stream);
  eid | fmt::Args{u8"memset failed, size={}", size};
}

auto mem_copy(void* dst, const void* src, usize size) -> void {
  const auto d = ::CUdeviceptr(dst);
  const auto s = ::CUdeviceptr(src);
  const auto eid = ::cuMemcpyAsync_ptsz(d, s, size, _tls_stream);
  eid | fmt::Args{u8"memcpy failed"};
}
#pragma endregion

#pragma region array
using arr_f = ::CUarray_format;

static auto fmt_size(arr_f f) -> usize {
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

auto arr_new(ArrXFmt xfmt, Dims dims, ArrFlag flags) -> arr_t {
  auto desc = ::CUDA_ARRAY3D_DESCRIPTOR{};
  desc.Width = dims[0];
  desc.Height = dims[1];
  desc.Depth = dims[2];
  desc.Flags = u32(flags);
  desc.NumChannels = xfmt._channels;
  desc.Format = arr_f(xfmt._fmt);

  arr_t arr = nullptr;
  const auto eid = ::cuArray3DCreate_v2(&arr, &desc);
  eid | fmt::Args{u8"create array failed, dims={}", dims};
  return arr;
}

auto arr_del(arr_t arr) -> void {
  if (arr == nullptr) return;
  const auto eid = ::cuArrayDestroy(arr);
  eid | fmt::Args{u8"destroy array failed"};
}

using arr_s = ::CUDA_ARRAY3D_DESCRIPTOR;
auto arr_res(arr_t arr) -> arr_s {
  auto res = arr_s{};
  const auto eid = ::cuArray3DGetDescriptor_v2(&res, arr);
  eid | fmt::Args{u8"array get desc failed"};
  return res;
}

enum class ArrCopy { Set, Get };
static void arr_cpy(arr_t arr, const void* ptr, ArrCopy mode) {
  const auto res = imp::arr_res(arr);
  const auto num_channels = cmp::max(res.NumChannels, 1u);
  const auto fmt_size = imp::fmt_size(res.Format);

  auto args = ::CUDA_MEMCPY3D{};
  args.WidthInBytes = fmt_size * num_channels * res.Width;
  args.Height = cmp::max(res.Height, usize(1));
  args.Depth = cmp::max(res.Depth, usize(1));

  if (mode == ArrCopy::Set) {
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

auto arr_set(arr_t arr, const void* buf) -> void {
  return imp::arr_cpy(arr, const_cast<void*>(buf), ArrCopy::Set);
}

void arr_get(arr_t arr, void* buf) {
  return imp::arr_cpy(arr, buf, ArrCopy::Get);
}
#pragma endregion

#pragma region texture
using res_f = ::CUresourceViewFormat_enum;

static auto _res_fmt(const CUDA_ARRAY3D_DESCRIPTOR& desc) -> res_f {
  auto res = res_f(0);

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

auto tex_new(arr_t arr, TexDesc desc) -> tex_t {
  ::CUDA_ARRAY3D_DESCRIPTOR arr_desc = {};
  ::cuArray3DGetDescriptor_v2(&arr_desc, arr) |
      fmt::Args{u8"array get desc failed"};

  auto res_desc = ::CUDA_RESOURCE_DESC{};
  res_desc.resType = ::CU_RESOURCE_TYPE_ARRAY;
  res_desc.res.array.hArray = arr;

  auto tex_desc = ::CUDA_TEXTURE_DESC{};
  tex_desc.addressMode[0] = ::CUaddress_mode(desc._addr);
  tex_desc.addressMode[1] = ::CUaddress_mode(desc._addr);
  tex_desc.addressMode[2] = ::CUaddress_mode(desc._addr);
  tex_desc.filterMode = ::CUfilter_mode(desc._filter);

  CUDA_RESOURCE_VIEW_DESC res_view = {};
  res_view.width = arr_desc.Width;
  res_view.height = arr_desc.Height;
  res_view.depth = arr_desc.Depth;

  if (arr_desc.Flags == u32(ArrFlag::Layered)) {
    res_view.firstLayer = 0;
    res_view.lastLayer = u32(arr_desc.Depth - 1);
  }
  res_view.format = imp::_res_fmt(arr_desc);

  unsigned long long tex;
  const auto eid = ::cuTexObjectCreate(&tex, &res_desc, &tex_desc, &res_view);
  eid | fmt::Args{u8"rc::cuda: texture create failed"};

  return tex_t(tex);
}

auto tex_del(tex_t tex) -> void {
  if (tex == 0) return;
  const auto eid = ::cuTexObjectDestroy(tex);
  eid | fmt::Args{u8"destroy texture failed"};
}

auto tex_arr(tex_t tex) -> arr_t {
  if (tex == tex_t(0)) return nullptr;
  auto res_desc = ::CUDA_RESOURCE_DESC{};

  const auto eid = ::cuTexObjectGetResourceDesc(&res_desc, u64(tex));
  eid | fmt::Args{u8"tex get desc failed"};
  return res_desc.res.array.hArray;
}

#pragma endregion

#pragma region mod
auto mod_new(const void* dat) -> mod_t {
  mod_t mod = nullptr;
  const auto eid = ::cuModuleLoadData(&mod, dat);
  eid | fmt::Args{u8"module load data failed"};
  return mod;
}

auto mod_del(mod_t mod) -> void {
  if (mod == nullptr) return;

  const auto eid = ::cuModuleUnload(mod);
  eid | fmt::Args{u8"module unload failed"};
}

auto mod_fun(mod_t mod, Str name) -> fun_t {
  auto cname = ffi::CString{name};

  fun_t fun = nullptr;
  const auto eid = ::cuModuleGetFunction(&fun, mod, cname);
  eid | fmt::Args{u8"cannot find func `{}`", name};
  return fun;
}

auto fun_run(fun_t f, FnDims dims, const void* args[]) -> void {
  const auto bx = u32(dims._blk[0]);
  const auto by = u32(dims._blk[1]);
  const auto bz = u32(dims._blk[2]);
  const auto tx = u32(dims._trd[0]);
  const auto ty = u32(dims._trd[1]);
  const auto tz = u32(dims._trd[2]);

  const auto m = 0;                         // shared memory
  const auto s = _tls_stream;               // stream
  const auto v = const_cast<void**>(args);  // args
  const auto e = nullptr;                   // extra args

  const auto eid = ::cuLaunchKernel_ptsz(f, bx, by, bz, tx, ty, tz, m, s, v, e);
  eid | fmt::Args{u8"launch failed, blks={}, trds={}", dims._blk, dims._trd};
}
#pragma endregion

}  // namespace rc::cuda::imp
