#pragma once

#include "sfc/core/reflect.h"
#include "sfc/core/fmts.h"

namespace sfc::fmt {

struct SBuf {
  char* _ptr;
  usize _cap;
  usize _len = 0;

 public:
  template <usize N>
  constexpr SBuf(char (&s)[N]) : _ptr{s}, _cap{N - 1} {}

  auto as_str() const -> Str {
    return Str{_ptr, _len};
  }

  void write_str(Str s) {
    if (s._len == 0 || _len + s._len > _cap) return;
    ptr::copy_nonoverlapping(s._ptr, _ptr + _len, s._len);
    _len += s._len;
  }
};

struct Write {
  using write_str_t = void(void*, Str);
  void* _obj;
  write_str_t* _write_str;

 public:
  template <class W>
  Write(W& w) : _obj{&w}, _write_str{ops::dyn_fn<&W::write_str>()} {}

  void write_str(Str s) {
    _write_str(_obj, s);
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

struct Formatter {
  Write _out;
  Spec _spec = {};

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
    } else if constexpr (requires { str::Str{val}; }) {
      this->pad(Str{val});
    } else if constexpr (requires { Slice{val}; }) {
      Slice{val}.fmt(*this);
    } else {
      fmt::Debug::fmt(val, *this);
    }
  }

  void write_arg(Spec spec, const auto& val) {
    _spec = spec;
    this->write_val(val);
  }

  void write_fmt(const Fmts& fmts, const auto&... args) {
    const auto xargs = Args{fmts, args...};
    xargs.fmt(*this);
  }

 public:
  auto debug_list() -> DebugList;
  auto debug_set() -> DebugSet;
  auto debug_map() -> DebugMap;

  auto debug_tuple(Str name) -> DebugTuple;
  auto debug_struct(Str name) -> DebugStruct;
};

class DebugList {
  Formatter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugList(Formatter& fmt);
  ~DebugList();
  DebugList(const DebugList&) = delete;

  void push(Str s);

  auto entry(const auto& value) -> DebugList& {
    this->push({});
    _fmt.write_val(value);
    return *this;
  }

  auto entries(auto&& iter) -> DebugList& {
    iter.for_each([&](auto&& val) { this->entry(val); });
    return *this;
  }
};

class DebugSet {
  Formatter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugSet(Formatter& fmt);
  ~DebugSet();
  DebugSet(const DebugSet&) = delete;

  void push(Str s);

  auto entry(const auto& value) -> DebugSet& {
    this->push({});
    _fmt.write_val(value);
    return *this;
  }

  auto entries(auto&& iter) -> DebugSet& {
    iter.for_each([&](auto&& val) { this->entry(val); });
    return *this;
  }
};

class DebugMap {
  Formatter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugMap(Formatter& fmt);
  ~DebugMap();
  DebugMap(const DebugMap&) = delete;

  void push(Str key, Str value);

  auto entry(Str key, const auto& value) -> DebugMap& {
    this->push(key, {});
    _fmt.write_val(value);
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
  Formatter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugTuple(Formatter& fmt);
  ~DebugTuple();
  DebugTuple(const DebugTuple&) = delete;

  void push(Str s);

  auto field(const auto& value) -> DebugTuple& {
    this->push("");
    _fmt.write_val(value);
    return *this;
  }
};

class DebugStruct {
  Formatter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugStruct(Formatter& fmt);
  ~DebugStruct();
  DebugStruct(const DebugStruct&) = delete;

  void push(Str key, Str value);
  auto field(Str key, const auto& value) -> DebugStruct& {
    this->push(key, {});
    _fmt.write_val(value);
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
