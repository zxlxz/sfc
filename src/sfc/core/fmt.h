#pragma once

#include "sfc/core/reflect.h"
#include "sfc/core/fmts.h"
#include "sfc/core/dyn.h"

namespace sfc::fmt {

struct DynWrite {
  class Self;
  Self& _self;
  void (*_write_str)(Self&, Str);

 public:
  template <class X>
  explicit DynWrite(X& x) : _self{dyn::cast<Self>(x)} {
    _write_str = [](Self& self, Str s) { (void)((X&)self).write_str(s); };
  }

 public:
  void write_str(Str s) {
    _write_str(_self, s);
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

class DebugBlock;
class DebugList;
class DebugSet;
class DebugMap;

class DebugTuple;
class DebugStruct;

class Formatter {
  DynWrite _out;
  Spec _spec = {};
  u32 _indent_level = 0;

 public:
  explicit Formatter(auto& out) : _out{DynWrite{out}} {}

 public:
  auto spec() const -> Spec {
    return _spec;
  }

  auto type() const -> char {
    return _spec._type;
  }

  auto width() const -> Option<u32> {
    if (!_spec._width) return {};
    return _spec._width;
  }

  auto precision() const -> Option<u32> {
    if (!_spec._point) return {};
    return _spec._precision;
  }

 public:
  void write_str(Str s) {
    if (s._len == 0) return;
    _out.write_str(s);
  }

  void write_char(char c) {
    _out.write_str({&c, 1});
  }

  void write_chars(char c, usize n);

  void pad(Str s);
  void pad_num(bool is_neg, Str num_str);

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

 public:
  friend class DebugBlock;
  auto debug_list() -> DebugList;
  auto debug_set() -> DebugSet;
  auto debug_map() -> DebugMap;

  auto debug_tuple(Str name) -> DebugTuple;
  auto debug_struct(Str name) -> DebugStruct;
};

class DebugBlock {
  Formatter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugBlock(Formatter& fmt, Str name = "");
  ~DebugBlock();
  DebugBlock(const DebugBlock&) = delete;

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
  DebugBlock _blk;

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
  DebugBlock _blk;

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
  DebugBlock _blk;

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
  DebugBlock _blk;

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
  DebugBlock _blk;

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

}  // namespace sfc::fmt
