#pragma once

#include <ustd/core/str.h>
#include <ustd/core/panic.h>
#include <ustd/core/trait.h>

namespace ustd::option
{

template<typename T, typename = $trait<T, void, $value, $class> >
class Option;

template<typename T>
class Option<T, void>
{
    enum class Type: u8 {
        Some,
        None,
    };
    Type    _type;

    Option($type<Type, Type::Some>)
        : _type(Type::Some) {
    }

    Option($type<Type, Type::None>)
        : _type(Type::None) {
    }
  public:
    // constructor: Some
    static fn Some() -> Option {
        return Option($type<Type, Type::Some>{});
    }

    // constructor: None
    static fn None() -> Option {
        return Option($type<Type, Type::None>{});
    }

    // property: is_some
    fn is_some() const noexcept -> bool {
        return _type == Type::Some;
    }

    // property: is_some
    fn is_none() const noexcept -> bool {
        return _type == Type::None;
    }

    // op[unsafe]: unpack
    fn operator*() const noexcept -> T {
        return T();
    }

    // op: |
    [[nodiscard]] fn operator |(Option optb) const noexcept -> Option {
        return _type == Type::Some ? *this : optb;
    }

    // op: &
    template<typename U>
    [[nodiscard]] fn operator &(Option<U> optb) const noexcept -> Option<U> {
        return _type == Type::None ? *this : optb;
    }

    // method: except
    template<typename ...Ts>
    fn except(str fmt, const Ts& ...ts) const -> T {
        if (_type != Type::Some) { panic(fmt, ts...); }
        return T();
    }

    // method: unwrap
    fn unwrap() const -> T {
        if (_type != Type::Some) { panic("type is none"); }
        return T();
    }

    // method: map
    template<class F, class U=decltype(declval<F>())>
    fn map(F&& f) const -> Option<U> {
        return _type == Type::Some ? Option<U>::Some(f()) : Option<U>::None();
    }

};

template<typename T>
class Option<T, $value>
{
    union {
        T   _val;
        u8  _nul;
    };

    enum class Type: u8 {
        Some,
        None,
    };
    Type    _type;
   
    template<class U>
    Option($type<Type, Type::Some>, U&& t)
        : _val(as_fwd<U>(t)), _type(Type::Some)
    {}

    Option($type<Type, Type::None>)
        : _nul(0), _type(Type::None)
    {}
  public:
    // constructor: Some
    template<class U>
    static fn Some(U&& t) -> Option {
        return Option($type<Type, Type::Some>{}, as_fwd<U>(t));
    }

    // constructor: None
    static fn None() -> Option {
        return Option($type<Type, Type::None>{});
    }

    // property: is_some
    fn is_some() const noexcept -> bool {
        return _type == Type::Some;
    }

    // property: is_some
    fn is_none() const noexcept -> bool {
        return _type == Type::None;
    }

    // op[unsafe]: unpack
    fn operator*() noexcept -> T& {
        return _val;
    }

    // op: |
    [[nodiscard]] fn operator |(Option<T> optb) const noexcept -> Option<T> {
        return _type == Type::Some ? *this : optb;
    }

    // op: &
    template<typename U>
    [[nodiscard]] fn operator &(Option<U> optb) const noexcept -> Option<U> {
        return _type == Type::None ? *this : optb;
    }

    // method: except
    template<typename ...Ts>
    fn except(str fmt, const Ts& ...ts) const -> T {
        if (_type != Type::Some) { panic(fmt, ts...); }
        return _val;
    }

    // method: unwrap
    fn unwrap() const -> T {
        if (_type != Type::Some) { panic("type is none"); }
        return _val;
    }

    // method: unwrap_or
    fn unwrap_or(T def) const noexcept -> T {
        return _type == Type::Some ? _val : def;
    }

    // method: unwrap_or_else
    template<class F>
    fn unwrap_or_else(F&& f) const noexcept -> T {
        return _type == Type::Some ? _val : f();
    }

    // method: map
    template<class F, class U=decltype(declval<F>(_val))>
    fn map(F&& f) const -> Option<U> {
        return _type == Type::Some ? Option<U>::Some(f(_val)) : Option<U>::None();
    }

};

template<typename T>
class Option<T, $class>
{
    union {
        T   _val;
        u8  _nul;
    };

    enum class Type: u8 {
        Some,
        None,
    };
    Type    _type;
   
    template<class U>
    Option($type<Type, Type::Some>, U&& t)
        : _val(as_fwd<U>(t)), _type(Type::Some)
    {}

    Option($type<Type, Type::None>)
        : _nul(0), _type(Type::None)
    {}

  public:
    // constructor: Some
    template<class U>
    static fn Some(U&& t) -> Option {
        return Option($type<Type, Type::Some>{}, as_fwd<U>(t));
    }

    // constructor: None
    static fn None() -> Option {
        return Option($type<Type, Type::None>{});
    }

    // property: is_some
    fn is_some() const noexcept -> bool {
        return _type == Type::Some;
    }

    // property: is_some
    fn is_none() const noexcept -> bool {
        return _type == Type::None;
    }

    // op[unsafe]: unpack
    fn operator*() noexcept -> T& {
        return _val;
    }

    // method: except
    template<typename ...Ts>
    fn except(str fmt, const Ts& ...ts)&& -> T {
        if (_type != Type::Some) { panic(fmt, ts...); }
        return as_mov(_val);
    }

    // method: unwrap
    fn unwrap() const -> T {
        if (_type != Type::Some) { panic("type is none"); }
        return as_mov(_val);
    }

    // method: unwrap_or
    fn unwrap_or(T def) const noexcept -> T {
        return _type == Type::Some ? as_mov(_val) : def;
    }

    // method: unwrap_or_else
    template<class F>
    fn unwrap_or_else(F&& f) && noexcept -> T {
        return _type == Type::Some ? as_mov(_val) : f();
    }

    // method: map
    template<class F, class U=decltype(declval<F>(as_mov(_val)))>
    fn map(F&& f) && -> Option<U> {
        using Option = Option<U>;
        return _type == Type::Some ? Option::Some(f(as_mov(_val))) : Option::None();
    }
};

}
