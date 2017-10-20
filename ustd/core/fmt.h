#pragma once

#include <ustd/core/string.h>
#include <ustd/core/panic.h>

namespace ustd::fmt
{

using ustd::string::String;
    
// [[fill]align][sign][width]['.'precision][type]
struct Formatter
{
    i8  fill ;  // [a-Z]?
    i8  align;  // [<>^]?
    i8  sign ;  // [+- ]?
    u8  width;  // [0-9]*
    u8  prec ;  // [0-9|*]*
    i8  type ;  // [?oxXpbeE]

   static fn from_str(str spec) -> Formatter;

    template<class T> 
    fn sfmt(String& outbuf, const T& val) const -> void {
        using U = $cond<$is<$struct, T> || $is<$class, T>, $class, void>;
        _sfmt_switch(outbuf, val, $type<U>{});
    }

    template<usize N>
    fn sfmt(String& outbuf, const char(&s)[N]) const -> void {
        sfmt_val(outbuf, str(s));
    }

    template<class T>
    fn sfmt(String& outbuf, const View<T>& s) const -> void {
        sfmt_view(outbuf, s);
    }

    template<class ...T>
    fn sfmt(String& outbuf, const Tuple<T...>& v) const -> void {
        sfmt_tuple(outbuf, v);
    }
  public:
   fn sfmt_val(String& outbuf, i8   val) const -> void;
   fn sfmt_val(String& outbuf, u8   val) const -> void;
   fn sfmt_val(String& outbuf, i16  val) const -> void;
   fn sfmt_val(String& outbuf, u16  val) const -> void;
   fn sfmt_val(String& outbuf, i32  val) const -> void;
   fn sfmt_val(String& outbuf, u32  val) const -> void;
   fn sfmt_val(String& outbuf, i64  val) const -> void;
   fn sfmt_val(String& outbuf, u64  val) const -> void;
   fn sfmt_val(String& outbuf, f32  val) const -> void;
   fn sfmt_val(String& outbuf, f64  val) const -> void;
   fn sfmt_val(String& outbuf, bool val) const -> void;
   fn sfmt_val(String& outbuf, str  val) const -> void;

    template<class T>
    fn sfmt_view(String& outbuf, const View<T>& v) const -> void {
        outbuf.push('[');
        for (let& t : v) {
            this->sfmt(outbuf, t);
            outbuf.push_str(", ");
        }
        outbuf.push(']');
    }

    fn sfmt_view(String& outbuf, const View<const char>& v) const -> void {
        sfmt_val(outbuf, str(v));
    }

    fn sfmt_view(String& outbuf, const View<char>& v) const  -> void {
        sfmt_val(outbuf, str(v));
    }

    fn sfmt_tuple(String& outbuf, const Tuple<>& v) const -> void {
        (void)v;
        outbuf.push_str("()");
    }

    template<class ...T>
    fn sfmt_tuple(String& outbuf, const Tuple<T...>& v) const -> void {
        enum { N = sizeof...(T) };

        outbuf.push('(');
        $for<N>([&](let idx) {
            sfmt(outbuf, v[idx]);
            if (!$is<decltype(idx), $usize<N-1> >) {
                outbuf.push_str(", ");
            }
        });
        outbuf.push(')');
    }
  private:
    template<class T>
    fn _sfmt_switch(String& outbuf, const T& val, $type<void>) const -> void {
        sfmt_val(outbuf, val);
    }

    template<class T>
    fn _sfmt_switch(String& outbuf, const T& val, $type<$class>) const -> void {
        val.sfmt(outbuf, *this);
    }
};

fn _sfmt_parse(String& outbuf, str& fmtstr, str *fmtspec, i32 *argidx) -> bool;

inline fn _sfmt_val(i32 idx, String& outbuf, str fmtstr) -> void {
    (void)idx;
    (void)outbuf;
    (void)fmtstr;
}

template<class T, typename ...U>
fn _sfmt_val(i32 idx, String& outbuf, str fmtstr, const T& t, const U& ...args) -> void {
    if (idx != 0) {
        _sfmt_val(idx - 1, outbuf, fmtstr, args...);
    }
    else {
        let fmtspec = Formatter::from_str(fmtstr);
        fmtspec.sfmt(outbuf, t);
    }
}

}

/* macros */
namespace ustd
{
fn println(str s) -> void;

template<typename ...T>
fn sformat(string::String& outbuf, str fmtstrs, const T& ...args) -> void {
    i32 arg_idx  = -1;
    str fmt_str = {};

    while(fmt::_sfmt_parse(outbuf, fmtstrs, &fmt_str, &arg_idx)) {
        fmt::_sfmt_val(arg_idx, outbuf, fmt_str, args...);
    }
}

template<class T, class ...U>
fn println(str fmtstr, const T& t, const U& ...u) -> void {
    vec::Vec<char, 4096> outbuf;
    sformat(outbuf, fmtstr, t, u...);
    println(outbuf);
}

template<typename ...T>
fn panic(str fmtstr, const T& ...args) -> void {
    println(fmtstr, args...);
    panic();
}

}
