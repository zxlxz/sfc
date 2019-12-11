#pragma once

#include "rc/cuda/mod.h"
#include "rc/math/mod.h"

struct CUctx_st;

namespace rc::cuda::imp {

using ctx_t = ::CUctx_st*;
using Dims = math::NDDims<3>;

auto dev_cnt() -> usize;
auto dev_raw(usize idx) -> dev_t;
void dev_sync();
auto dev_name(dev_t dev) -> String;
auto dev_arch(dev_t dev) -> Device::Arch;
auto dev_ctx(dev_t dev) -> ctx_t;
void set_ctx(ctx_t ctx);

auto stream_new(u32 flags) -> thr_t;
void stream_del(thr_t thr);
auto stream_default() -> thr_t;
void stream_sync(thr_t thr);
void set_stream(thr_t thr);

auto mem_new(usize size, MemType type) -> void*;
void mem_del(void* p);
void mem_set(void* ptr, u8 val, usize size);
void mem_copy(void* dst, const void* src, usize size);

auto arr_new(ArrXFmt xfmt, Dims dims, ArrFlag flag) -> arr_t;
void arr_del(arr_t arr);
void arr_set(arr_t arr, const void* buf);
void arr_get(arr_t arr, void* buf);

auto tex_new(arr_t arr, TexDesc desc) -> tex_t;
void tex_del(tex_t tex);
auto tex_arr(tex_t arr) -> arr_t;

auto mod_new(void* dat) -> mod_t;
void mod_del(mod_t mod);
auto mod_fun(mod_t mod, Str name) -> fun_t;
void fun_run(fun_t fun, FnDims dims, void* args[]);

}  // namespace rc::cuda::imp
