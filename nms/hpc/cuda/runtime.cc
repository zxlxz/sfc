#define module cumodule
#include <nms/hpc/cuda/cuda.h>
#undef module

#include <nms/core.h>
#include <nms/io.h>
#include <nms/util/library.h>
#include <nms/hpc/cuda/runtime.h>
#include <nms/hpc/cuda/engine.h>

namespace nms::hpc::cuda
{

#pragma region library

#define NMS_CUDA_DEF(F)                                                                                     \
    F(cuGetErrorName),      F(cuDeviceGetCount),        F(cuInit),                                          \
    F(cuCtxCreate_v2),      F(cuCtxSetCurrent),         F(cuCtxSynchronize),                                \
    F(cuStreamCreate),      F(cuStreamDestroy_v2),      F(cuStreamSynchronize),                             \
    F(cuMemAlloc_v2),       F(cuMemFree_v2),            F(cuMemHostAlloc),              F(cuMemFreeHost),   \
    F(cuArray3DCreate_v2),  F(cuArrayDestroy),          F(cuArray3DGetDescriptor_v2),                       \
    F(cuMemcpy),            F(cuMemcpyHtoD_v2),         F(cuMemcpyDtoH_v2),                                 \
    F(cuMemcpyAsync),       F(cuMemcpyHtoDAsync_v2),    F(cuMemcpyDtoHAsync_v2),                            \
    F(cuTexObjectCreate),   F(cuTexObjectDestroy),                                                          \
    F(cuModuleLoadData),    F(cuModuleUnload),                                                              \
    F(cuModuleGetFunction), F(cuModuleGetGlobal_v2),    F(cuLaunchKernel)

enum CudaLibTable
{
#define NMS_CUDA_IDX(name) $##name
    NMS_CUDA_DEF(NMS_CUDA_IDX)
#undef  NMS_CUDA_IDX
};

static Library::Function cudaFun(u32 id) {
    static Library lib("NVCUDA.DLL");

    static Library::Function funcs[] = {
#define NMS_CUDA_FUN(name) lib[cstr(#name)]
        NMS_CUDA_DEF(NMS_CUDA_FUN)
#undef  NMS_CUDA_FUN
    };

    auto ret = funcs[id];

    if (!ret) {
        throw Exception(CUDA_ERROR_NOT_INITIALIZED);
    }
    return ret;
}

#define NMS_CUDA_DO(name)   static_cast<decltype(name)*>(cudaFun($##name))

#pragma endregion

#pragma region exception
void Exception::format(String& buf) const {
    const char* errmsg = nullptr;
    NMS_CUDA_DO(cuGetErrorName)(static_cast<CUresult>(id_), &errmsg);
    sformat(buf, "nms.hpc.cuda: error({}), {}", id_, cstr(errmsg));
}

/**
* log message and emit exception when cuda runtime failes.
*/
template<u32 N>
__forceinline void operator||(cudaError_enum eid,  const char(&msg)[N]) {
    if (eid == 0) {
        return;
    }
    io::log::error(msg);
    throw Exception(eid);
}

#pragma endregion

#pragma region device
i32         gDevID      = -1;
CUctx_st*   gDevCtx[32];

void driver_init() {
    static const auto stat = [] {
        const auto init_stat = NMS_CUDA_DO(cuInit)(0);
        if (init_stat != 0) {
            return init_stat;
        }

        return CUDA_SUCCESS;
    }();

    if (stat != 0) {
        throw Exception(stat);
    }
}

void device_sync() {
    NMS_CUDA_DO(cuCtxSynchronize)() || "nms.hpc.cuda.Device.sync: failed";
}

NMS_API u32 Device::count() {
    driver_init();

    static i32 dev_count = 0;

    static auto static_init = [&] {

        const auto dev_stat = NMS_CUDA_DO(cuDeviceGetCount)(&dev_count);
        if (dev_stat != 0) {
            return dev_stat;
        }

        for (auto i = 0; i < dev_count; ++i) {
            const auto ctx_stat = NMS_CUDA_DO(cuCtxCreate_v2)(&gDevCtx[i], 0, i);
            if (ctx_stat != 0) {
                io::log::error("nms.hpc.cuda: create context failed.");
                return ctx_stat;
            }
        }

        const auto ctx_stat = NMS_CUDA_DO(cuCtxSetCurrent)(gDevCtx[0]);
        if (ctx_stat != 0) {
            io::log::error("nms.hpc.cuda: set context failed.");
            return ctx_stat;
        }

        return CUDA_SUCCESS;
    }();
    (void)static_init;

    return u32(dev_count);
}

NMS_API void Device::sync() const {
    auto oid = gDevID;
    auto nid = id_;

    if (oid!=nid) {
        NMS_CUDA_DO(cuCtxSetCurrent)(gDevCtx[nid]) || "nms.hpc.cuda: cannot set context";
    }

    device_sync();

    if (oid!=nid) {
        NMS_CUDA_DO(cuCtxSetCurrent)(gDevCtx[oid])|| "nms.hpc.cuda: cannot set context";
    }
}

#pragma endregion

#pragma region stream
NMS_API Stream::Stream(nullptr_t)
    : id_(nullptr) {
    (void)Device::count();
}

NMS_API Stream::Stream()
    : id_(nullptr) {
    NMS_CUDA_DO(cuStreamCreate)(&id_, 0) || "nms.hpc.cuda.Stream: create stream failed";
}

NMS_API Stream::~Stream() {
    if (id_ == nullptr) {
        return;
    }
    NMS_CUDA_DO(cuStreamDestroy_v2)(id_) || "nms.hpc.cuda.Stream: destroy stream failed";
}

NMS_API void Stream::sync() const {
    NMS_CUDA_DO(cuStreamSynchronize)(id_) || "nms.hpc.cuda.Stream: sync failed";
}

#pragma endregion

#pragma region memory
NMS_API void* _mnew(u64 size) {
    if (size == 0) {
        return nullptr;
    }
    (void)Device::count();

    CUdeviceptr ptr = 0;
    NMS_CUDA_DO(cuMemAlloc_v2)(&ptr, size) || "nms.hpc.cuda.mnew: failed.";
    return reinterpret_cast<void*>(ptr);
}

NMS_API void _mdel(void* ptr) {
    if (ptr == nullptr) {
        return;
    }
    NMS_CUDA_DO(cuMemFree_v2)(reinterpret_cast<CUdeviceptr>(ptr)) || "nms.hpc.cuda.mdel: failed";
}

NMS_API void* _hnew(u64 size) {
    if (size == 0) {
        return nullptr;
    }
    (void)Device::count();

    void* ptr = nullptr;
    NMS_CUDA_DO(cuMemHostAlloc)(&ptr, size, 0) || "nms.hpc.cuda.hnew: failed";
    return ptr;
}

NMS_API void _hdel(void* ptr) {
    if (ptr == nullptr) {
        return;
    }
    NMS_CUDA_DO(cuMemFreeHost)(ptr) || "nms.hpc.cuda.hdel: failed";
}

static CUarray_format_enum arr_fmt(char type, u32 size) {
    if (type == 'i') {
        switch (size) {
        case 1: return CU_AD_FORMAT_SIGNED_INT8;
        case 2: return CU_AD_FORMAT_SIGNED_INT16;
        case 4: return CU_AD_FORMAT_SIGNED_INT32;
        default:break;
        }
    }
    if (type == 'u') {
        switch (size) {
        case 1: return CU_AD_FORMAT_UNSIGNED_INT8;
        case 2: return CU_AD_FORMAT_UNSIGNED_INT16;
        case 4: return CU_AD_FORMAT_UNSIGNED_INT32;
        default:break;
        }
    }
    if (type == 'f') {
        switch (size) {
        case 4: return CU_AD_FORMAT_FLOAT;
        default:break;
        }
    }
    throw Exception(CUDA_ERROR_INVALID_VALUE);
}

NMS_API arr_t arr_new(char type, u32 size, u32 channels, u32 rank, const u32 dims[]) {
    arr_t arr = nullptr;
    CUDA_ARRAY3D_DESCRIPTOR desc = {
        rank > 0 ? dims[0] : 0, // width
        rank > 1 ? dims[1] : 0, // height
        rank > 2 ? dims[2] : 0, // depth
        arr_fmt(type, size),    // format
        channels,               // chanels
        0,                      // flags
    };

    const auto stat = NMS_CUDA_DO(cuArray3DCreate_v2)(&arr, &desc);
    void(stat || "nms.hpc.cuda.arr_new: cuda array create failed.");

    return arr;
}

NMS_API void arr_del(arr_t arr) {
    const auto stat = NMS_CUDA_DO(cuArrayDestroy)(arr);
    void(stat || "nms.hpc.cuda.arr_del: cuda array destory failed.");
}

NMS_API void _mcpy(void* dst, const void* src, u64 size, Stream& stream) {
    if (dst == nullptr || src == nullptr || size == 0) {
        return;
    }
    auto dptr = reinterpret_cast<CUdeviceptr>(dst);
    auto sptr = reinterpret_cast<CUdeviceptr>(src);
    NMS_CUDA_DO(cuCtxSynchronize)() || "nms.hpc.cuda.Device.sync: failed"; 

    auto sid = stream.id();
    auto ret = sid == nullptr
        ? NMS_CUDA_DO(cuMemcpy)(dptr, sptr, size_t(size))
        : NMS_CUDA_DO(cuMemcpyAsync)(dptr, sptr, size_t(size), sid);

    ret || "nms.hpc.cuda.mcpy: failed";
}

NMS_API void _h2dcpy(void* dst, const void* src, u64 size, Stream& stream) {
    if (dst == nullptr || src == nullptr || size == 0) {
        return;
    }
    auto sid = stream.id();
    auto ret = sid == nullptr
        ? NMS_CUDA_DO(cuMemcpyHtoD_v2)(reinterpret_cast<CUdeviceptr>(dst), src, size)
        : NMS_CUDA_DO(cuMemcpyHtoDAsync_v2)(reinterpret_cast<CUdeviceptr>(dst), src, size, sid);

    ret || "nms.hpc.cuda.h2dcpy: failed";
}

NMS_API void _d2hcpy(void* dst, const void* src, u64 size, Stream& stream) {
    if (dst == nullptr || src == nullptr || size == 0) {
        return;
    }
    auto sid = stream.id();
    auto ret = sid == nullptr
        ? NMS_CUDA_DO(cuMemcpyDtoH_v2)(dst, reinterpret_cast<CUdeviceptr>(src), size)
        : NMS_CUDA_DO(cuMemcpyDtoHAsync_v2)(dst, reinterpret_cast<CUdeviceptr>(src), size, sid);

    ret || "nms.hpc.cuda.d2hcpy: failed";
}

#pragma endregion

#pragma region texture

static CUresourceViewFormat_enum mkTexFormat(CUarray_format_enum fmt, u32 n) {
    using T = CUresourceViewFormat_enum;

    const u32 offset = n == 1 ? 0 : n == 2 ? 1 : n == 4 ? 2 : 0;

    switch (fmt) {
    case CU_AD_FORMAT_SIGNED_INT8:      return T(offset + CU_RES_VIEW_FORMAT_SINT_1X8  );
    case CU_AD_FORMAT_SIGNED_INT16:     return T(offset + CU_RES_VIEW_FORMAT_SINT_1X16 );
    case CU_AD_FORMAT_SIGNED_INT32:     return T(offset + CU_RES_VIEW_FORMAT_SINT_1X32 );
    case CU_AD_FORMAT_UNSIGNED_INT8:    return T(offset + CU_RES_VIEW_FORMAT_UINT_1X8  );
    case CU_AD_FORMAT_UNSIGNED_INT16:   return T(offset + CU_RES_VIEW_FORMAT_UINT_1X16 );
    case CU_AD_FORMAT_UNSIGNED_INT32:   return T(offset + CU_RES_VIEW_FORMAT_UINT_1X32 );
    case CU_AD_FORMAT_FLOAT:            return T(offset + CU_RES_VIEW_FORMAT_FLOAT_1X32);
    default: break;
    }

    return CU_RES_VIEW_FORMAT_NONE;
}

NMS_API u64 tex_new(arr_t arr, TexAddressMode addres_mode, TexFilterMode filter_mode) {
    CUtexObject tex = 0;

    CUDA_ARRAY3D_DESCRIPTOR arr_desc = {};
    CUDA_RESOURCE_DESC      res_desc = {};
    CUDA_TEXTURE_DESC       tex_desc = {};
    CUDA_RESOURCE_VIEW_DESC view_desc = {};

    res_desc.resType = CU_RESOURCE_TYPE_ARRAY;
    res_desc.res.array.hArray = arr;

    tex_desc.addressMode[0] = CUaddress_mode(addres_mode);
    tex_desc.addressMode[1] = CUaddress_mode(addres_mode);
    tex_desc.addressMode[2] = CUaddress_mode(addres_mode);
    tex_desc.filterMode = CUfilter_mode(filter_mode);

    NMS_CUDA_DO(cuArray3DGetDescriptor_v2)(&arr_desc, arr);
    view_desc.format = mkTexFormat(arr_desc.Format, arr_desc.NumChannels);
    view_desc.width = arr_desc.Width;
    view_desc.height = arr_desc.Height;
    view_desc.depth = arr_desc.Depth;

    const auto stat = NMS_CUDA_DO(cuTexObjectCreate)(&tex, &res_desc, &tex_desc, &view_desc);
    void(stat || "nms.hpc.cuda.tex_new: cuda texture object create failed");

    return tex;
}

NMS_API void tex_del(u64 obj) {
    const auto stat = NMS_CUDA_DO(cuTexObjectDestroy)(obj);
    void(stat || "nms.hpc.cuda.tex_del: cuda texture object destroy failed");
}

#pragma endregion

#pragma region module
NMS_API Module::Module(StrView ptx)
    : module_(nullptr)
{
    driver_init();

    auto eid = NMS_CUDA_DO(cuModuleLoadData)(&module_, ptx.data());
    if (eid != 0) {
        io::log::error("nms.hpc.cuda.Module.build: load ptx failed.");
    }
}

NMS_API Module::~Module() {
    if (module_ == nullptr) {
        return;
    }

    NMS_CUDA_DO(cuModuleUnload)(module_);
    module_ = nullptr;
}

NMS_API Module::sym_t Module::get_symbol(StrView name) const {
    if (module_ == nullptr) {
        return nullptr;
    }

    auto cname = name.data();

    CUdeviceptr ptr = 0;
    size_t      size = 0;
    const auto  ret = NMS_CUDA_DO(cuModuleGetGlobal_v2)(&ptr, &size, module_, cname);
    if (ret != 0 || ptr == 0) {
        io::log::error("nms.hpc.cuda.Module.getArg: cannot get arg => cufun_{}_{}", name);
        throw Exception(ret);
    }
    return reinterpret_cast<sym_t>(ptr);
}

NMS_API void Module::set_symbol(sym_t data, const void* value, u32 size) const {
    _h2dcpy(data, value, size, Stream::global());
}


NMS_API Module::fun_t Module::get_kernel(StrView name) const {
    if (module_ == nullptr) {
        return nullptr;
    }

    auto cname = name.data();

    CUfunc_st* fun = nullptr;
    auto ret = NMS_CUDA_DO(cuModuleGetFunction)(&fun, module_, cname);
    if (ret != 0) {
        io::log::error("nms.hpc.cuda.Program.get_kernel: cannot get {}", name);
        throw Exception(ret);
    }

    return reinterpret_cast<fun_t>(fun);
}

NMS_API Module::fun_t Module::get_kernel(u32 index) const {
    char name[64];
    auto count = snprintf(name, sizeof(name), "nms_hpc_cuda_foreach_%u", index);
    auto func = get_kernel(StrView(name, { u32(count) }));
    return func;
}

NMS_API void Module::run_kernel(fun_t kernel, const void* kernel_args[], u32 rank, const u32 dims[], Stream& stream) const {
    u32 block_dim[3] = {
        rank == 1 ? 256u : rank == 2 ? 16u : rank == 3 ? 8u : 8u,
        rank == 1 ? 001u : rank == 2 ? 16u : rank == 3 ? 8u : 8u,
        rank == 1 ? 001u : rank == 2 ? 01u : rank == 3 ? 8u : 8u,
    };

    u32 grid_dim[3] = {
        rank > 0 ? (dims[0] + block_dim[0] - 1) / block_dim[0] : 1,
        rank > 1 ? (dims[1] + block_dim[1] - 1) / block_dim[1] : 1,
        rank > 2 ? (dims[2] + block_dim[2] - 1) / block_dim[2] : 1
    };

    const auto shared_mem_bytes   = 0;
    const auto h_stream           = stream.id();
    const auto extra              = nullptr;

    const auto stat = NMS_CUDA_DO(cuLaunchKernel)(kernel, grid_dim[0], grid_dim[1], grid_dim[2], block_dim[0], block_dim[1], block_dim[2], shared_mem_bytes, h_stream, const_cast<void**>(kernel_args), extra);
    void(stat || "nms.hpc.cuda.invoke: failed.");
}

NMS_API Module& gModule() {
    static StrView ptx_src = [=] {
        const io::Path src_path("#/config/nms.hpc.cuda.program.cu");
        const io::Path ptx_path("#/config/nms.hpc.cuda.program.ptx");
        auto& program = gProgram();

        // if not exists, try save
        if (!io::exists(ptx_path)) {
            program.compile();

            io::TxtFile src_file(src_path, io::TxtFile::Write);
            src_file.write(program.src());

            io::TxtFile ptx_file(ptx_path, io::TxtFile::Write);
            ptx_file.write(program.ptx());
        }

        static auto ptx = io::loadString(ptx_path);
        return StrView(ptx);
    }();

    static Module value(ptx_src);
    return value;
}

#pragma endregion

}
