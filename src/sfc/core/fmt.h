#pragma once

#include "sfc/core/str.h"
#include "sfc/core/fmts.h"
#include "sfc/core/trait.h"
#include "sfc/core/reflect.h"

namespace sfc::fmt {

class Write {
 public:
  struct VTbl {
    void (*write_str)(void*, Str);
  };

  template <class X>
  static constexpr auto vtbl = VTbl{
      .write_str = [](void* x, auto... u) { return ((X*)(x))->write_str(u...); },
  };

  void write_str(this Dyn<Write>& self, Str s) {
    return (self._vtbl)->write_str(self._impl, s);
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
    _len = new_len < _len ? new_len : _len;
  }

  void write_str(Str s) {
    if (_len >= N) return;
    const auto n = s._len < (N - _len) ? s._len : (N - _len);
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

class DebugList;
class DebugSet;
class DebugMap;
class DebugTuple;
class DebugStruct;

class Formatter {
  friend struct fmt::Args;
  Dyn<Write> _out;
  Spec _spec = {};
  u16 _depth = 0;
  u16 _max_depth = 100;

 public:
  explicit Formatter(auto& out, Spec spec = {}) : _out{out}, _spec{spec} {}

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

Args::Args(const Fmts& fmts, const auto&... args) : _fmts{fmts}, _args{&args...} {
  static_assert(sizeof...(args) <= kMaxLen, "fmt::Args: too many arguments, should(<= 16)");

#if !defined(__INTELLISENSE__) && !defined(__clang_analyzer__)
  using args_t = decltype(tuple::bind(args...));
  _fmt = [](const Args& self, fmt::Formatter& f) {
    const auto& fmts = self._fmts;
    const auto& args = *(const args_t*)self._args;
    const auto spec = f._spec;
    args.for_each([&, idx = 0U](const auto& val) mutable {
      if (idx >= fmts._len) return;
      const auto [fill, spec] = fmts._vals[idx++];
      f._spec = spec;
      f.write_str({fill._ptr, fill._len});
      f.write_val(val);
    });
    f.write_str({fmts._tail._ptr, fmts._tail._len});
    f._spec = spec;
  };
#endif
}

// macro: write!(out, arg...)
void write(auto& out, const fmt::Fmts& fmts, const auto&... args) {
  Formatter{out}.write_fmt(fmts, args...);
}

void writeln(auto& out, const fmt::Fmts& fmts, const auto&... args) {
  fmt::write(out, fmts, args...);
  out.write_str("\n");
}

}  // namespace sfc::fmt
