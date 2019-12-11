#pragma once

#include "rc/core/str.h"

namespace rc::fmt {

using slice::Slice;
using Bytes = Slice<const u8>;

// [[fill]align][sign]['#'][width]['.' precision][type]
struct Style {
  u8 _fill = 0;    // *
  u8 _align = 0;   // <>^=
  u8 _sign = 0;    // +-
  u8 _prefix = 0;  // #

  u8 _width = 0;  // [0-9]*
  u8 _point = 0;  // [.]
  u8 _prec = 0;   // [0-9]*
  u8 _type = 0;   // *

  pub static auto from(Str s) -> Style;

  auto fill() const noexcept -> u8 { return _fill == u8'\0' ? u8' ' : _fill; }

  auto align() const noexcept -> u8 { return _align; }

  auto sign() const noexcept -> u8 { return _sign; }

  auto alternate() const noexcept -> u8 { return _prefix == u8'#'; }

  auto type() const noexcept -> u8 { return _type; }

  auto width() const noexcept -> u32 { return _width; }

  auto precision() const noexcept -> Option<u32> {
    if (_point != u8'.') {
      return {};
    } else {
      return {_prec};
    }
  }

  auto prefix() const noexcept -> Str {
    if (_prefix != u8'#') {
      return u8"";
    }
    switch (_type) {
      case u8'o':
        return u8"0";
      case u8'b':
        return u8"0b";
      case u8'B':
        return u8"0B";
      case u8'x':
        return u8"0x";
      case u8'X':
        return u8"0X";
      default:
        return u8"";
    }
  }
};

#pragma region Formatter

template <class T>
struct Display;

template <class... T>
struct Args;

template <class Out>
struct Formatter {
  Out& _out;
  Style _style = {};
  u32 _depth = 0;

  Formatter(Out& out) noexcept : _out{out} {}
  Formatter(const Formatter&) = delete;

  constexpr auto fill() const noexcept -> u8 { return _style.fill(); }

  constexpr auto align() const noexcept -> u8 { return _style.align(); }

  constexpr auto sign() const noexcept -> u8 { return _style.sign(); }

  constexpr auto type() const noexcept -> u8 { return _style.type(); }

  constexpr auto width() const noexcept -> u32 { return _style.width(); }

  constexpr auto precision() const noexcept -> Option<u32> {
    return _style.precision();
  }

  constexpr auto prefix() const noexcept -> Str { return _style.prefix(); }

  void write_str(Str s) {
    const auto buf = s.as_bytes();
    _out.write(buf);
  }

  void write_chr(u8 c) {
    const auto buf = Slice<const u8>{&c, 1};
    _out.write(buf);
  }

  void write_chs(usize n, u8 c) {
    const auto buf = Slice<const u8>{&c, 1};
    for (usize i = 0; i < n; ++i) {
      _out.write(buf);
    }
  }

  template <class T>
  void write_val(const T& val) {
    fmt::Display<T>{val}.fmt(*this);
  }

  template <class... T>
  void write_fmt(const Args<T...>& val) {
    val.fmt(*this);
  }

  template <class... U>
  void write(const U&... args) {
    if constexpr (sizeof...(U) == 1) {
      this->write_val(args...);
    } else if constexpr (sizeof...(U) > 1) {
      const auto argv = fmt::Args{args...};
      this->write_fmt(argv);
    }
  }

  template <class... U>
  void writeln(const U&... args) {
    if constexpr (sizeof...(U) != 0) {
      this->write(args...);
    }
    this->write_chr(u8'\n');
  }

  void pad(Str buf) {
    const auto write_len = buf.len();
    const auto style_len = _style.width();

    if (style_len <= write_len) {
      this->write_str(buf);
      return;
    }

    const auto fill_cnt = style_len - write_len;
    const auto fill_chr = u8(_style._fill == 0 ? u8' ' : _style._fill);

    switch (_style.align()) {
      case '<':
        this->write_str(buf);
        this->write_chs(fill_cnt, fill_chr);
        break;

      default:
      case '>':
        this->write_chs(fill_cnt, fill_chr);
        this->write_str(buf);
        break;

      case '^':
        this->write_chs((fill_cnt + 0) / 2, fill_chr);
        this->write_str(buf);
        this->write_chs((fill_cnt + 1) / 2, fill_chr);
    }
  }

  void pad_integral(bool is_neg, Str buf) {
    auto sign = is_neg ? Str(u8"-") : _style._sign ? Str(u8"+") : Str(u8"");
    auto prefix = _style.prefix();
    auto write_len = sign.len() + prefix.len() + buf.len();
    auto style_len = _style.width();

    if (style_len <= write_len) {
      this->write_str(sign);
      this->write_str(prefix);
      this->write_str(buf);
      return;
    }

    const auto fill_cnt = style_len - write_len;
    const auto fill_chr =
        u8(_style._fill != u8'\0' ? _style._fill
                                  : _style._prefix == u8'#' ? u8'0' : u8' ');

    switch (_style.align()) {
      case u8'<':
        this->write_str(sign);
        this->write_str(prefix);
        this->write_str(buf);
        this->write_chs(fill_cnt, fill_chr);
        break;
      case u8'>':
      default:
        this->write_chs(fill_cnt, fill_chr);
        this->write_str(sign);
        this->write_str(prefix);
        this->write_str(buf);
        break;
      case u8'^':
        this->write_chs((fill_cnt + 0) / 2, fill_chr);
        this->write_str(sign);
        this->write_str(prefix);
        this->write_str(buf);
        this->write_chs((fill_cnt + 1) / 2, fill_chr);
        break;
      case u8'=':
        this->write_str(sign);
        this->write_str(prefix);
        this->write_chs(fill_cnt, fill_chr);
        this->write_str(buf);
        break;
    }
  }

  void indent() {
    this->write_chr(u8'\n');
    this->write_chs(usize(_depth) * 2u, ' ');
  }

  struct DebugTuple;
  auto debug_tuple() -> DebugTuple;

  struct DebugList;
  auto debug_list() -> DebugList;

  struct DebugDict;
  auto debug_dict() -> DebugDict;

  struct DebugStruct;
  auto debug_struct() -> DebugStruct;
};

template <class I>
Formatter(I&)->Formatter<I>;
#pragma endregion

#pragma region Args
struct Argf {
  Str _str;

  auto as_str() const noexcept -> Str { return _str; }

  auto find(u8 c, usize p, usize n) -> usize {
    for (; p < n; ++p) {
      if (_str.get_unchecked(p) == c) {
        const auto q = p + 1;
        if (q >= n || _str.get_unchecked(q) != c) break;
        ++p;
      }
    }
    return p;
  }

  template <class Out>
  auto next(Formatter<Out>& f) -> Style {
    const auto n = _str.len();
    const auto p = this->find(u8'{', 0, n);
    const auto q = this->find(u8'}', p, n);
    if (p == n || q == n) {
      rc::panic(u8"rc::fmt::Args: parse failed");
    }
    const auto s = _str.slice_unchecked(p + 1, q);

    f.write_str(_str.slice_unchecked(0, p));
    _str = _str.slice_from(q + 1);
    return Style::from(s);
  }

  template <class Out, class T>
  void fmt(Formatter<Out>& f, const T& t) {
    f._style = this->next(f);
    fmt::Display<T>{t}.fmt(f);
  }
};

template <class... T>
struct Args {
  static constexpr usize SIZE = sizeof...(T);

  Argf _argf;
  Tuple<const T&...> _args;

  explicit Args(Str argf, const T&... args) noexcept
      : _argf{argf}, _args{args...} {}

  template <class Out>
  void fmt(Formatter<Out>& f) const {
    auto v = _argf;
    if constexpr (SIZE > 0) v.fmt(f, _args._0);
    if constexpr (SIZE > 1) v.fmt(f, _args._1);
    if constexpr (SIZE > 2) v.fmt(f, _args._2);
    if constexpr (SIZE > 3) v.fmt(f, _args._3);
    if constexpr (SIZE > 4) v.fmt(f, _args._4);
    if constexpr (SIZE > 5) v.fmt(f, _args._5);
    if constexpr (SIZE > 6) v.fmt(f, _args._6);
    if constexpr (SIZE > 7) v.fmt(f, _args._7);
    if constexpr (SIZE > 8) v.fmt(f, _args._8);
    if constexpr (SIZE > 9) v.fmt(f, _args._9);
    f.write_str(v.as_str());
  }
};

template <class... T>
Args(Str argf, const T&... args)->Args<T...>;
#pragma endregion

#pragma region Debug
template <class Buf>
struct Formatter<Buf>::DebugTuple {
  Formatter<Buf>& _super;
  usize _len = 0;

  explicit DebugTuple(Formatter& formatter) : _super{formatter} {
    _super.write_str(u8"(");
    _super._depth += 1;
  }

  ~DebugTuple() {
    _super._depth -= 1;
    _super.write_str(u8")");
  }

  DebugTuple(const DebugTuple&) = delete;

  template <class T>
  void entry(const T& val) {
    if (_len != 0) {
      _super.write_str(_super._style._type == u8'?' ? Str(u8",\n")
                                                    : Str(u8", "));
    } else if (_super._style._type == u8'?') {
      _super.indent();
    }
    Display<T>{val}.fmt(_super);
    _len += 1;
  }
};

template <class Buf>
auto Formatter<Buf>::debug_tuple() -> DebugTuple {
  return DebugTuple{*this};
}

template <class Buf>
struct Formatter<Buf>::DebugList {
  Formatter<Buf>& _super;
  usize _len = 0;

  explicit DebugList(Formatter<Buf>& formatter) : _super{formatter} {
    _super.write_str(u8"[");
    _super._depth += 1;
  }

  ~DebugList() {
    _super._depth -= 1;
    _super.write_str(u8"]");
  }

  DebugList(const DebugList&) = delete;

  template <class T>
  void entry(const T& val) {
    if (_len != 0) {
      _super.write_str(_super._style._type == u8'?' ? Str(u8",\n")
                                                    : Str(u8", "));
    } else if (_super._style._type == u8'?') {
      _super.indent();
    }
    Display<T>{val}.fmt(_super);
    _len += 1;
  }
};

template <class Buf>
auto Formatter<Buf>::debug_list() -> DebugList {
  return DebugList{*this};
}

template <class Buf>
struct Formatter<Buf>::DebugDict {
  Formatter<Buf>& _super;
  usize _len = 0;

  explicit DebugDict(Formatter<Buf>& formatter) : _super{formatter} {
    _super.write_str(u8"{");
    _super._depth += 1;
  }

  ~DebugDict() {
    _super._depth -= 1;
    _super.write_str(u8"}");
  }

  DebugDict(const DebugDict&) = delete;

  template <class T>
  void field(Str key, const T& val) {
    if (_len != 0) {
      _super.write_str(_super._style._type == u8'?' ? Str(u8",\n")
                                                    : Str(u8", "));
    } else if (_super._style._type == u8'?') {
      _super.indent();
    }
    _super.write_str(key);
    _super.write_str(u8": ");
    Display<T>{val}.fmt(_super);
    _len += 1;
  }
};

template <class Buf>
auto Formatter<Buf>::debug_dict() -> DebugDict {
  return DebugDict{*this};
}

template <class Buf>
struct Formatter<Buf>::DebugStruct {
  Formatter<Buf>& _super;
  usize _len = 0;

  DebugStruct(Formatter<Buf>& formatter) : _super{formatter} {
    _super.write_str(u8"{");
    _super._depth += 1;
  }

  ~DebugStruct() {
    _super._depth -= 1;
    _super.write_str(u8"}");
  }

  DebugStruct(const DebugStruct&) = delete;

  template <class T>
  void field(Str key, const T& val) {
    if (_len != 0) {
      _super.write_str(_super._style._type == '?' ? Str(u8",\n") : Str(u8", "));
    } else if (_super._style._type == '?') {
      _super.indent();
    }
    _super.write_str(u8"\"");
    _super.write_str(key);
    _super.write_str(u8"\": ");
    Display<T>{val}.fmt(_super);
    _len += 1;
  }
};

template <class Buf>
auto Formatter<Buf>::debug_struct() -> DebugStruct {
  return DebugStruct{*this};
}
#pragma endregion

#pragma region Display
template <class Self>
struct Display {
  const Self& _self;

  template <class Buf>
  void fmt(Formatter<Buf>& formatter) const {
    _self.fmt(formatter);
  }
};

template <>
struct Display<bool> {
  const bool& _self;

  template <class Out>
  void fmt(Formatter<Out>& f) const {
    const auto s = _self ? Str{u8"true"} : Str{u8"false"};
    f.write_str(s);
  }
};

template <>
struct Display<Str> {
  const Str& _self;

  template <class Out>
  void fmt(Formatter<Out>& f) const {
    if (f.type() != u8'?') {
      f.pad(_self);
    } else {  // debug?
      f.write_str(u8"\"");
      f.write_str(_self);
      f.write_str(u8"\"");
    }
  }
};

template <class T>
struct Display<Slice<T>> {
  const Slice<T>& _self;

  template <class Out>
  void fmt(Formatter<Out>& f) const {
    auto dbg_list = f.debug_list();
    for (auto& element : _self) {
      dbg_list.entry(element);
    }
  }
};

template <class... T>
struct Display<Tuple<T...>> {
  const Tuple<T...>& _self;

  template <class Out>
  void fmt(Formatter<Out>& f) const {
    auto dbg_tuple = f.debug_tuple();
    if constexpr (sizeof...(T) > 0) dbg_tuple.entry(_self._0);
    if constexpr (sizeof...(T) > 1) dbg_tuple.entry(_self._1);
    if constexpr (sizeof...(T) > 2) dbg_tuple.entry(_self._2);
    if constexpr (sizeof...(T) > 3) dbg_tuple.entry(_self._3);
    if constexpr (sizeof...(T) > 4) dbg_tuple.entry(_self._4);
    if constexpr (sizeof...(T) > 5) dbg_tuple.entry(_self._5);
    if constexpr (sizeof...(T) > 6) dbg_tuple.entry(_self._6);
    if constexpr (sizeof...(T) > 7) dbg_tuple.entry(_self._7);
    if constexpr (sizeof...(T) > 8) dbg_tuple.entry(_self._8);
    if constexpr (sizeof...(T) > 9) dbg_tuple.entry(_self._9);
  }
};

template <class T>
struct Display<Option<T>> {
  const Option<T>& _self;

  template <class Out>
  void fmt(Formatter<Out>& f) const {
    if (_self.is_none()) {
      f.pad(u8"None()");
    } else {
      f.write(u8"Some({})", _self.as_some());
    }
  }
};

template <class T, usize N>
struct Display<T[N]> {
  const T (&_self)[N];

  template <class Out>
  void fmt(Formatter<Out>& f) const {
    if constexpr (rc::is_char<T>()) {
      f.pad(Str{_self});
    } else {
      auto dbg_list = f.debug_list();
      for (const auto& item : _self) {
        dbg_list.entry(item);
      }
    }
  }
};
#pragma endregion

#pragma region funcs
template <class T>
struct IntoFormatter {
  Formatter<T> _0;
  auto as_mut() -> Formatter<T>& { return _0; }
};

template <class T>
struct IntoFormatter<Formatter<T>> {
  Formatter<T>& _0;
  auto as_mut() -> Formatter<T>& { return _0; }
};

template<class T>
IntoFormatter(T&) -> IntoFormatter<T>;

template <class Out, class... U>
void write(Out& buf, const U&... args) {
  IntoFormatter<Out>{buf}.as_mut().write(args...);
}

template <class Out, class... U>
void writeln(Out& out, const U&... args) {
  IntoFormatter<Out>{out}.as_mut().writeln(args...);
}
#pragma endregion

}  // namespace rc::fmt
