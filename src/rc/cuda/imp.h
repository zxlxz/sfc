#pragma once

#include "rc/cuda/mod.h"
#include "rc/math/mod.h"

struct CUstream_st;
struct CUarray_st;
struct CUmod_st;
struct CUfunc_st;

namespace rc::cuda::imp {

using mod_t = ::CUmod_st*;
using thr_t = ::CUstream_st*;
using fun_t = ::CUfunc_st*;
using arr_t = ::CUarray_st*;
using tex_t = u64;

using Dims = math::NDDims<3>;

auto dev_cnt() -> usize;
auto dev_set(usize idx) -> void;
auto dev_syn() -> void;

auto thr_new() -> thr_t;
auto thr_del(thr_t thr) -> void;
auto thr_set(thr_t thr) -> void;
auto thr_syn(thr_t thr) -> void;

auto mem_new(usize size, MemType type) -> void*;
auto mem_del(void* p) -> void;
auto mem_set(void* ptr, u8 val, usize size) -> void;
auto mem_cpy(void* dst, const void* src, usize size) -> void;

auto arr_new(ArrXFmt xfmt, Dims dims, ArrFlag flag) -> arr_t;
void arr_del(arr_t arr);
void arr_set(arr_t arr, const void* buf);
void arr_get(arr_t arr, void* buf);

auto tex_new(arr_t arr, TexDesc desc) -> tex_t;
auto tex_del(tex_t tex) -> void;
auto tex_arr(tex_t arr) -> arr_t;

auto mod_new(void* dat) -> mod_t;
auto mod_del(mod_t mod) -> void;
auto mod_fun(mod_t mod, Str name) -> fun_t;
auto fun_run(fun_t fun, FnDims dims, void* args[]) -> void;

}  // namespace rc::cuda::imp
