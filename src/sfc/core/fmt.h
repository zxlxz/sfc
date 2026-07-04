#pragma once

#include "sfc/core/reflect.h"
#include "sfc/core/fmts.h"

namespace sfc::fmt {

class Formatter;

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
  Write(W& w) : _obj{&w}, _write_str{([](void* x, Str s) { ((W*)x)->write_str(s); })} {}

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

  static void fmt(trait::enum_ auto val, auto& f) {
    using I = __underlying_type(decltype(val));
    constexpr auto kTypeName = reflect::type_name<decltype(val)>();
    if constexpr (requires { to_str(val); }) {
      f.write_str(to_str(val));
    } else {
      f.write_fmt("{}({})", kTypeName, __builtin_bit_cast(I, val));
    }
  }
};

class DebugList;
class DebugSet;
class DebugMap;
class DebugTuple;
class DebugStruct;

class Formatter {
  friend struct fmt::Fmts;

  Write _out;
  Spec _spec = {};

 public:
  Formatter(auto& w) : _out{w}, _spec{} {}

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
  void write_str(Str s);
  void write_char(char c);
  void write_chars(char c, usize n);

  void pad(Str s);
  void pad_num(bool is_neg, Str num_str);

  void write_fmt(const Fmts& fmts, const auto&... args) {
    const auto xargs = Args{fmts, args...};
    xargs.fmt(*this);
  }

 public:
  auto debug_list() -> DebugList;
  auto debug_set() -> DebugSet;
  auto debug_map() -> DebugMap;
  auto debug_tuple() -> DebugTuple;
  auto debug_struct() -> DebugStruct;
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
    Display::fmt(value, _fmt);
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
    Display::fmt(value, _fmt);
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
    Display::fmt(value, _fmt);
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
    Display::fmt(value, _fmt);
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
    Display::fmt(value, _fmt);
    return *this;
  }
};

void Display::fmt(const auto& self, auto&& formatter) {
  if constexpr (requires { self.fmt(formatter); }) {
    self.fmt(formatter);
  } else if constexpr (requires { str::Str{self}; }) {
    formatter.pad(Str{self});
  } else if constexpr (requires { Slice{self}; }) {
    Slice{self}.fmt(formatter);
  } else {
    fmt::Debug::fmt(self, formatter);
  }
}

void Fmts::format_imp(fmt::Formatter& f, u32 idx, const auto& val) const {
  if (idx >= _cnt) {
    return;
  }

  const auto fill = _fills[idx];
  const auto spec = _specs[idx];
  f._spec = spec;
  f.write_str({fill._ptr, fill._len});
  Display::fmt(val, f);
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
