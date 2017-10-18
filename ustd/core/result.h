#pragma once

#include <ustd/core/option.h>

namespace ustd::result
{

using option::Option;

template<typename T, typename E, typename = $trait<T, void, $value, $class>, typename = $trait<E, void, $value, $class> > 
class Result;

template<>
class Result<void, void, void, void >
{
    enum class Type: u8 { Ok, Err };

    Type    _type;

    Result(Type type): _type(type) 
    {}

  public:
    // constructor: Ok
    static fn Ok() -> Result {
        return Result(Type::Ok);
    }

    // constructor: Err
    static fn Err() -> Result {
        return Result(Type::Err);
    }

    // property: is_ok
    fn is_ok() const noexcept -> bool {
        return _type == Type::Ok;
    }

    // property: is_err
    fn is_err() const noexcept -> bool {
        return _type == Type::Err;
    }

    // method: and_then
    template<class F, typename U = decltype(declval<F>()())>
    fn and_then(F&& op) const ->Result<U, void> {
        using UResult = Result<U, void>;
        return _type == Type::Ok ? UResult::Ok(op()) : UResult::Err();
    }

    // method: or_else
    template<class O, typename F = decltype(declval<O>()())>
    fn or_else(O&& op) const ->Result<void, F> {
        using UResult = Result<void, F>;
        return _type == Type::Ok ? UResult::Ok() : UResult::Err(op());
    }

    template<typename ...Targ>
    fn except(str fmt, const Targ& ...args) const -> void {
        if (_type != Type::Ok) { panic(fmt, args...); }
    }

    template<typename F, typename U = decltype(declval<F>()()) >
    fn map(F&& op) const -> Result<U, void> {
        using UResult = Result<U, void>;
        return _type == Type::Ok ? op() : UResult::Err();
    }
};

template<typename T, typename E>
class Result<T, E, void, $value>
{
    enum class Type: u8 { Ok, Err };

    union {
        u8  _nul;
        E   _err;
    };
    Type    _type;

    Result($type<Type, Type::Ok>)
        : _nul(0), _type(Type::Ok) {
    }

    template<class F>
    Result($type<Type, Type::Err>, F&& e)
        : _err(as_fwd<F>(e)), _type(Type::Err) {
    }

  public:
    // constructor: Ok
    static fn Ok() -> Result {
        return Result($type<Type, Type::Ok>{});
    }

    // constructor: Err
    template<class F>
    static fn Err(F&& e) -> Result {
        return Result($type<Type, Type::Err>{}, as_fwd<F>(e));
    }

    // property: is_ok
    fn is_ok() const noexcept -> bool {
        return _type == Type::Ok;
    }

    // property: is_err
    fn is_err() const noexcept -> bool {
        return _type == Type::Err;
    }

    // method: ok, discard error value
    fn ok() const -> Option<void> {
        using Option = Option<void>;
        return _type == Type::Ok ? Option::Some() : Option::None();
    }

    // method: err, discard success value
    fn err() const -> Option<E> {
        using Option = Option<E>;
        return _type == Type::Ok ? Option::None() : Option::Some();
    }

    // method: and_then
    template<class F, typename U = decltype(declval<F>()())>
    fn and_then(F&& op) const ->Result<U, E> {
        return _and_then(op, $type<U>{});
    }

    // method: or_else
    template<class O, typename F = decltype(declval<O>()(_err))>
    fn or_else(O&& op) const ->Result<void, F> {
        return _or_else(op, $type<F>{});
    }

    fn unwrap() const -> void {
        if (_type != Type::Err) { ustd::panic("type != Type::Ok"); }
        return;
    }

    template<typename ...Targ>
    fn except(str fmt, const Targ& ...args) const -> void {
        if (_type != Type::Ok) { panic(fmt, args...); }
    }

    template<typename F, typename U = decltype(declval<F>()()) >
    fn map(F&& op) const -> Result<U, E> {
        using Result = Result<U, E>;
        return _type == Type::Ok ? Result::Ok(op()) : Result::Err(_err);
    }

  private:
      // method-impl: and_then
      template<class F>
      fn _and_then(F&& op, $type<void>) const ->Result<void, E> {
          using Result = Result<void, E>;
          return _type == Type::Ok ? (op(), Result::Ok()) : Result::Err(_err);
      }

      // method-impl: and_then
      template<class F, typename U>
      fn _and_then(F&& op, $type<U>) const ->Result<U, E> {
          using Result = Result<U, E>;
          return _type == Type::Ok ? Result::Ok(op()) : Result::Err(_err);
      }

      // method-impl: or_else
      template<class O>
      fn _or_else(O&& op,$type<void>) const ->Result<void, void> {
          using Result = Result<void, void>;
          return _type == Type::Ok ? Result::Ok() : (op(_err), Result::Err());
      }

      // method-impl: or_else
      template<class O, typename F>
      fn _or_else(O&& op, $type<F>) const ->Result<void, F> {
          using Result = Result<void, F>;
          return _type == Type::Ok ? Result::Ok() : Result::Err(op(_err));
      }
};

template<typename T, typename E>
class Result<T, E, $value, $value>
{
    enum class Type: u8 { Ok, Err };

    union {
        T   _ok;
        E   _err;
    };
    Type  _type;

    template<class U>
    Result($type<Type, Type::Ok>, U&& t)
        : _ok(as_fwd<U>(t)), _type(Type::Ok)
    {}

    template<class F>
    Result($type<Type, Type::Err>, F&& e)
        : _err(as_fwd<F>(e)), _type(Type::Err)
    {}

  public:    
    // constructor: Ok
    template<class U>
    static fn Ok(U&& t) -> Result {
        return Result($type<Type, Type::Ok>{}, as_fwd<U>(t));
    }

    // constructor: Err
    template<class F>
    static fn Err(F&& e) -> Result {
        return Result($type<Type, Type::Err>{}, as_fwd<F>(e));
    }

    // property: is_ok
    fn is_ok() const noexcept -> bool {
        return _type == Type::Ok;
    }

    // property: is_err
    fn is_err() const noexcept -> bool {
        return _type == Type::Err;
    }

    // method: ok, discard error value
    fn ok() const -> Option<T> {
        using Option = Option<T>;
        return _type == Type::Ok ? Option::Some(_ok) : Option::None();
    }

    // method: err, discard success value
    fn err() const -> Option<E>  {
        using Option = Option<E>;
        return _type == Type::Err ? Option::Some(_err) : Option::None();
    }

    fn unwrap() const -> T {
        if (_type != Type::Err) { 
            ustd::panic("type != Type::Ok");
        }
        return _ok;
    }

    fn unwrap_or(T def) const -> T {
        return _type == Type::Ok ? _ok : def;
    }

    template<typename ...Targ>
    fn except(str fmt, const Targ& ...args) const -> T {
        if (_type != Type::Ok)  { panic(fmt, args...); }
        return _ok;
    }

    template<typename F, typename U = decltype(declval<F>()(_ok)) >
    fn map(F&& op) const -> Result<U, E> {
        using Result = Result<U, E>;
        return _type == Type::Ok ? Result::Ok(op(_ok)) : Result::Err(_err);
    }
};

template<typename T, typename E>
class Result<T, E, $class, $value>
{
    enum class Type: u8 { _, Ok, Err };

    union {
        u8  _nul;
        T   _ok;
        E   _err;
    };
    Type  _type;

    template<class U>
    Result($type<Type, Type::Ok>, U&& t)
        : _ok(as_fwd<U>(t)), _type(Type::Ok)
    {}

    template<class F>
    Result($type<Type, Type::Err>, F&& e)
        : _err(as_fwd<F>(e)), _type(Type::Err)
    {}

  public:
    Result(Result&& other) noexcept : _nul(0), _type(other._type) {
        other._type = Type::_;
        if (_type == Type::Ok)  { $new(&_ok,  as_mov(other._ok )); }
        if (_type == Type::Err) { $new(&_err, as_mov(other._err)); }
    }

    ~Result() {}
   
    Result(const Result&)         = delete;
    fn operator=(const Result&)  = delete;

    // constructor: Ok
    template<class U>
    static fn Ok(U&& t) -> Result {
        return Result($type<Type, Type::Ok>{}, as_fwd<U>(t));
    }

    // constructor: Err
    template<class F>
    static fn Err(F&& e) -> Result {
        return Result($type<Type, Type::Err>{}, as_fwd<F>(e));
    }

    // property: is_ok
    fn is_ok() const noexcept -> bool {
        return _type == Type::Ok;
    }

    // property: is_err
    fn is_err() const noexcept -> bool {
        return _type == Type::Err;
    }

    // method: ok, discard error value
    fn ok() && -> Option<T> {
        using Option = Option<T>;
        return _type == Type::Ok ? Option::Some(as_mov(_ok)) : Option::None();
    }

    // method: err, discard success value
    fn err()&& -> Option<E>  {
        using Option = Option<T>;
        return _type == Type::Ok ? Option::None() : Option::Some(_err);
    }

    // method: and_then
    template<class F, typename U = decltype(declval<F>()(as_mov(_ok)))>
    fn and_then(F&& op) && -> Result<U, E> {
        using Result = Result<U, E>;
        return _type == Type::Ok ? Result::Ok(op(as_mov(_ok))) : Result::Err(_err);
    }

    // method: or_else
    template<class O, typename F = decltype(declval<O>()(_err))>
    fn or_else(O&& op) && ->Result<T, F> {
        using Result = Result<T, F>;
        return _type == Type::Ok ? Result::Ok(as_mov(_ok)) : Result::Err(op(_err));
    }

    fn unwrap()&& -> T {
        if (_type != Type::Ok) {
            ustd::panic("type != Type::Ok");
        }
        return as_mov(_ok);
    }

    fn unwrap_or(T def)&& -> T {
        return _type == Type::Ok ? as_mov(_ok) : def;
    }

    template<typename ...Targ>
    fn except(str fmt, const Targ& ...args)&& -> T {
        if (_type != Type::Ok)  { panic(fmt, args...); }
        return _ok;
    }

    template<typename F, typename U = decltype(declval<F>()(op(as_mov(_ok)))) >
    fn map(F&& op)&& -> Result<U, E> {
        using Result = Result<U, E>;
        return _type == Type::Ok ? Result::Ok(op(as_mov(_ok))) : Result::Err(_err);
    }
};

}
