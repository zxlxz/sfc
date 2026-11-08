#pragma once

#include "sfc/core/str.h"
#include "sfc/core/fmts.h"
#include "sfc/core/trait.h"
#include "sfc/core/reflect.h"

namespace sfc::fmt {

class Formatter;

struct Write {
  struct Impl;
  struct VTbl {
    void (*write_str)(void*, Str);
  };

  template <class X>
  static constexpr auto vtbl = VTbl{
      .write_str = [](void* x, Str s) { return ((X*)x)->write_str(s); },
  };
};

struct Write::Impl {
  const VTbl* _vtbl;
  void* _impl;

 public:
  void write_str(Str s) {
    return (_vtbl->write_str)(_impl, s);
  }
};

struct Buf {
  Slice<char> _buf;
  usize _len = 0;

 public:
  auto as_str() const -> Str;
  void truncate(usize new_len);
  void write_str(Str s) noexcept;
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

  static void fmt(decltype(nullptr) val, Formatter& f);
  static void fmt(const void* val, Formatter& f);
  static void fmt(const char* val, Formatter& f);

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

struct DebugList;
struct DebugSet;
struct DebugMap;
struct DebugTuple;
struct DebugStruct;

class Formatter {
  Dyn<Write> _out;
  Spec _spec = {};
  u16 _depth = 0;

 public:
  explicit Formatter(auto& out) : _out{out} {}
  ~Formatter() = default;

  Formatter(const Formatter&) = delete;
  Formatter& operator=(const Formatter&) = delete;

 public:
  auto spec() const -> Spec;
  auto type() const -> char;
  auto width() const -> Option<u32>;
  auto precision() const -> Option<u32>;
  auto depth() const -> u32;

 public:
  auto write_str(Str s) -> Formatter&;
  auto write_char(char c) -> Formatter&;
  auto write_chars(char c, usize n) -> Formatter&;

  void pad(Str s);
  void pad_num(bool is_neg, Str num_str);

 public:
  struct Block;
  auto debug_list() -> DebugList;
  auto debug_set() -> DebugSet;
  auto debug_map() -> DebugMap;

  auto debug_tuple(Str name) -> DebugTuple;
  auto debug_struct(Str name) -> DebugStruct;

 public:
  auto write_val(const auto& val) -> Formatter& {
    if constexpr (requires { val.fmt(*this); }) {
      val.fmt(*this);
    } else if constexpr (requires { val->fmt(*this); }) {
      val->fmt(*this);
    } else {
      fmt::Debug::fmt(val, *this);
    }
    return *this;
  }

  auto write_arg(Spec spec, const auto& arg) -> Formatter& {
    mem::swap(_spec, spec);
    this->write_val(arg);
    mem::swap(_spec, spec);
    return *this;
  }

  auto write_fmt(const Fmts& fmts, const auto&... args) -> Formatter& {
    const auto xargs = Args{fmts, args...};
    xargs.fmt(*this);
    return *this;
  }
};

struct Formatter::Block {
  Formatter& _fmt;
  u32 _fields = 0;
  u32 _indent_size = 2U;

 public:
  auto item(Str open) -> Formatter&;
  void finish(Str open, Str end);

  void write_val(const auto& val) {
    _fmt.write_val(val);
  }
};

struct DebugList {
  Formatter::Block _blk;

 public:
  auto entry(const auto& value) -> DebugList& {
    _blk.item("[").write_val(value);
    return *this;
  }

  auto entries(auto&& iter) -> DebugList& {
    iter.for_each([&](auto&& val) { this->entry(val); });
    return *this;
  }

  void finish() {
    _blk.finish("[", "]");
  }
};

struct DebugTuple {
  Formatter::Block _blk;

 public:
  auto field(const auto& value) -> DebugTuple& {
    _blk.item("(").write_val(value);
    return *this;
  }

  void finish() {
    _blk.finish("(", ")");
  }
};

struct DebugSet {
  Formatter::Block _blk;

 public:
  auto entry(const auto& value) -> DebugSet& {
    _blk.item("{").write_val(value);
    return *this;
  }

  auto entries(auto&& iter) -> DebugSet& {
    iter.for_each([&](auto&& val) { this->entry(val); });
    return *this;
  }

  void finish() {
    _blk.finish("{", "}");
  }
};

struct DebugMap {
  Formatter::Block _blk;

 public:
  auto key(Str key) -> DebugMap&;

  auto value(const auto& val) -> DebugMap& {
    _blk.write_val(val);
    return *this;
  }

  auto entry(Str key, const auto& value) -> DebugMap& {
    this->key(key);
    this->value(value);
    return *this;
  }

  auto entries(auto&& iter) -> DebugMap& {
    iter.for_each([&](const auto& item) {
      const auto& [k, v] = item;
      this->entry(k, v);
    });
    return *this;
  }

  void finish() {
    _blk.finish("{", "}");
  }
};

struct DebugStruct {
  Formatter::Block _blk;

 public:
  auto key(Str key) -> DebugStruct&;

  auto value(const auto& value) -> DebugStruct& {
    _blk.write_val(value);
    return *this;
  }

  auto field(Str key, const auto& value) -> DebugStruct& {
    this->key(key);
    this->value(value);
    return *this;
  }

  void finish() {
    _blk.finish("{", "}");
  }
};

Args::Args(const Fmts& fmts, const auto&... args) : _fmts{fmts}, _args{&args...} {
  static_assert(sizeof...(args) <= kMaxLen, "fmt::Args: too many arguments, should(<= 16)");

#if !defined(__INTELLISENSE__) && !defined(__clang_analyzer__)
  using args_t = decltype(tuple::bind(args...));
  _fmt = [](const Args& self, fmt::Formatter& f) {
    const auto& fmts = self._fmts;
    const auto& args = *(const args_t*)self._args;
    args.for_each([&, idx = 0U](const auto& val) mutable {
      if (idx >= fmts._len) return;
      const auto [fill, spec] = fmts._vals[idx++];
      f.write_str({fill._ptr, fill._len});
      f.write_arg(spec, val);
    });
    f.write_str({fmts._tail._ptr, fmts._tail._len});
  };
#endif
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
