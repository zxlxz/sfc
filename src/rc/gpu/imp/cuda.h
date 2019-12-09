#pragma once

#include "rc/core.h"

struct CUstream_st;
struct CUarray_st;
struct CUmod_st;
struct CUfunc_st;

namespace rc::gpu::cuda {

using mod_t = ::CUmod_st*;
using thr_t = ::CUstream_st*;
using fun_t = ::CUfunc_st*;
using arr_t = ::CUarray_st*;
using tex_t = u64;

enum class MemType {
  Host,
  Device,
};

enum class ArrFlag {
  Default = 0x0,
  Layered = 0x1,
  Surface = 0x2,
  Cubemap = 0x4,
  TexGather = 0x8,
};

enum class ArrKind {
  UInt,
  SInt,
  Float,
};

struct ArrType {
  ArrKind _kind;
  usize _size;
  u32 _channels;
};

enum class ArrCopy { P2A, A2P };

using ArrDims = Slice<const usize>;

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

using Extent = Tuple<usize, usize, usize>;

auto _dev_cnt() -> usize;
auto _dev_set(usize idx) -> void;
auto _dev_syn() -> void;

auto _thr_new() -> thr_t;
auto _thr_del(thr_t thr) -> void;
auto _thr_set(thr_t thr) -> void;
auto _thr_syn(thr_t thr) -> void;

auto _mem_new(usize size, MemType type) -> void*;
auto _mem_del(void* p, MemType type) -> void;
auto _mem_set(void* ptr, u8 val, usize size) -> void;
auto _mem_cpy(void* dst, void* src, usize size) -> void;

auto _arr_new(ArrType type, Extent dims, ArrFlag flag) -> arr_t;
auto _arr_del(arr_t arr) -> void;
auto _arr_cpy(arr_t arr, void* ptr, ArrCopy mode) -> void;

auto _tex_new(arr_t arr, TexAddr addr, TexFilter filter) -> tex_t;
auto _tex_del(tex_t tex) -> void;
auto _tex_arr(tex_t arr) -> arr_t;

auto _mod_new(void* dat) -> mod_t;
auto _mod_del(mod_t mod) -> void;
auto _mod_fun(mod_t mod, Str name) -> fun_t;
auto _fun_run(fun_t fun, Extent blks, Extent trds, void* args[]) -> void;

}  // namespace rc::gpu::cuda
