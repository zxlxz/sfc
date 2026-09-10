#pragma once

#include "sfc/core/cmp.h"
#include "sfc/core/fmts.h"
#include "sfc/core/trait.h"
#include "sfc/core/reflect.h"

namespace sfc::fmt {

class DynWrite final {
  void* _self;
  void (*_write_str)(void*, Str);

 public:
  template <trait::not_<DynWrite> X>
  DynWrite(X& x) : _self{&x}, _write_str{[](void* p, Str s) { (void)((X*)p)->write_str(s); }} {}

 public:
  void write_str(Str s) {
    _write_str(_self, s);
  }
};

template <usize N>
class Buf {
  char _buf[N];
  usize _len = 0;

 public:
  auto capacity() const -> usize {
    return N;
  }

  auto len() const -> usize {
    return _len;
  }

  auto as_str() const -> Str {
    return Str{_buf, _len};
  }

 public:
  void truncate(usize new_len) {
    _len = (cmp::min)(new_len, _len);
  }

  void write_str(Str s) {
    if (_len >= N) return;
    const auto n = (cmp::min)(N - _len, s._len);
    __builtin_memcpy(_buf + _len, s._ptr, n);
    _len += n;
  }
};

struct Debug {
  static void fmt(bool val, Formatter& f);
  static void fmt(char val, Formatter& f);
  static void fmt(char16_t val, Formatter& f);
  static void fmt(char32_t val, Formatter& f);

  static void fmt(unsigned int val, Formatter& f);
  static void fmt(unsigned long val, Formatter& f);
  static void fmt(unsigned long long val, Formatter& f);

  static void fmt(int val, Formatter& f);
  static void fmt(long val, Formatter& f);
  static void fmt(long long val, Formatter& f);

  static void fmt(float val, Formatter& f);
  static void fmt(double val, Formatter& f);

  static void fmt(const void* val, Formatter& f);

  template <trait::class_ T>
  static void fmt([[maybe_unused]] const T& val, auto& f) {
    if constexpr (requires { val.fmt(f); }) {
      val.fmt(f);
    } else {
      static_assert(__is_empty(T));
      f.write_fmt("{}()", reflect::type_name<T>());
    }
  }

  template <class T, usize N>
  static void fmt(const T (&arr)[N], auto& f) {
    if constexpr (requires { Str{arr}; }) {
      Str{arr}.fmt(f);
    } else {
      Slice{arr}.fmt(f);
    }
  }

  template <trait::enum_ T>
  static void fmt(T val, auto& f) {
    using I = __underlying_type(T);
    if constexpr (requires { to_str(val); }) {
      const auto s = to_str(val);
      f.write_str(s);
    } else {
      const auto name = reflect::type_name<T>();
      f.write_fmt("{}({})", name, I(val));
    }
  }
};

class DebugList;
class DebugSet;
class DebugMap;
class DebugTuple;
class DebugStruct;

class Formatter {
  DynWrite _out;
  Spec _spec = {};
  u16 _depth = 0;
  u16 _max_depth = 100;

 public:
  explicit Formatter(DynWrite out) : _out{out} {}

 public:
  auto spec() const -> Spec;
  auto type() const -> char;
  auto width() const -> Option<u32>;
  auto precision() const -> Option<u32>;
  auto depth() const -> u32;
  void set_max_depth(u32 max_depth);

 public:
  void write_str(Str s);
  void write_char(char c);
  void write_chars(char c, usize n);

  void pad(Str s);
  void pad_num(bool is_neg, Str num_str);

 public:
  class Block;
  auto debug_list() -> DebugList;
  auto debug_set() -> DebugSet;
  auto debug_map() -> DebugMap;

  auto debug_tuple(Str name) -> DebugTuple;
  auto debug_struct(Str name) -> DebugStruct;

 public:
  void write_val(const auto& val) {
    if constexpr (requires { val.fmt(*this); }) {
      val.fmt(*this);
    } else {
      fmt::Debug::fmt(val, *this);
    }
  }

  void write_arg(Spec spec, const auto& val) {
    auto old_spec = _spec;
    _spec = spec;
    this->write_val(val);
    _spec = old_spec;
  }

  void write_fmt(const Fmts& fmts, const auto&... args) {
    const auto xargs = Args{fmts, args...};
    xargs.fmt(*this);
  }
};

class Formatter::Block {
  Formatter& _fmt;
  u32 _cnt = 0;
  u32 _indent_size = 2U;

 public:
  explicit Block(Formatter& fmt, Str name = "");
  ~Block();
  Block(const Block&) = delete;

 public:
  void open(Str begin);
  void finish(Str end);
  void next();

  void write_key(Str key, char type = 0);

  void write_val(const auto& val) {
    _fmt.write_val(val);
  }
};

class DebugList {
  Formatter::Block _blk;

 public:
  explicit DebugList(Formatter& fmt);
  ~DebugList();

  auto entry(const auto& value) -> DebugList& {
    _blk.next();
    _blk.write_val(value);
    return *this;
  }

  auto entries(auto&& iter) -> DebugList& {
    iter.for_each([&](auto&& val) { this->entry(val); });
    return *this;
  }
};

class DebugSet {
  Formatter::Block _blk;

 public:
  explicit DebugSet(Formatter& fmt);
  ~DebugSet();
  DebugSet(const DebugSet&) = delete;

 public:
  auto entry(const auto& value) -> DebugSet& {
    _blk.next();
    _blk.write_val(value);
    return *this;
  }

  auto entries(auto&& iter) -> DebugSet& {
    iter.for_each([&](auto&& val) { this->entry(val); });
    return *this;
  }
};

class DebugMap {
  Formatter::Block _blk;

 public:
  explicit DebugMap(Formatter& fmt);
  ~DebugMap();
  DebugMap(const DebugMap&) = delete;

 public:
  auto entry(Str key, const auto& value) -> DebugMap& {
    _blk.next();
    _blk.write_key(key, '"');
    _blk.write_val(value);
    return *this;
  }

  auto entries(auto&& iter) -> DebugMap& {
    iter.for_each([&](const auto& item) {
      const auto& [k, v] = item;
      this->entry(k, v);
    });
    return *this;
  }
};

class DebugTuple {
  Formatter::Block _blk;

 public:
  explicit DebugTuple(Formatter& fmt, Str name);
  ~DebugTuple();
  DebugTuple(const DebugTuple&) = delete;

 public:
  auto field(const auto& value) -> DebugTuple& {
    _blk.next();
    _blk.write_val(value);
    return *this;
  }
};

class DebugStruct {
  Formatter::Block _blk;

 public:
  explicit DebugStruct(Formatter& fmt, Str name);
  ~DebugStruct();
  DebugStruct(const DebugStruct&) = delete;

 public:
  auto field(Str key, const auto& value) -> DebugStruct& {
    _blk.next();
    _blk.write_key(key);
    _blk.write_val(value);
    return *this;
  }
};

void Fmts::format_imp(fmt::Formatter& f, const auto& args) const {
  tuple::for_each_idx(args, [&](u32 idx, const auto& val) {
    if (idx >= _cnt) return;
    f.write_str({_fills[idx]._ptr, _fills[idx]._len});
    f.write_arg(_specs[idx], val);
  });
  f.write_str({_tail._ptr, _tail._len});
}

// macro: write!(out, arg...)
void write(auto& out, const fmt::Fmts& fmts, const auto&... args) {
  if constexpr (requires { out.write_fmt(fmts, args...); }) {
    out.write_fmt(fmts, args...);
  } else {
    Formatter{out}.write_fmt(fmts, args...);
  }
}

void writeln(auto& out, const fmt::Fmts& fmts, const auto&... args) {
  fmt::write(out, fmts, args...);
  out.write_str("\n");
}

}  // namespace sfc::fmt
