#pragma once

#include <nms/core/base.h>

namespace nms
{

template<class T>               constexpr bool $is_enum             = __is_enum(T);
template<class T>               constexpr bool $is_class            = __is_class(T);
template<class T>               constexpr bool $is_pod              = __is_pod(T);
template<class T>               constexpr bool $is_union            = __is_union(T);
template<class T>               constexpr bool $is_empty            = __is_empty(T);
template<class F, class    T>   constexpr bool $is_convert_to       = __is_convertible_to(F, T);
template<class T, class    F>   constexpr bool $is_assignable       = __is_assignable(T, F);
template<class B, class    T>   constexpr bool $is_base_of          = __is_base_of(B, T);

template<class T, class ...X>   constexpr bool $is_constructable    = __is_constructible(T, X...);
template<class T, class ...X>   constexpr bool $is_trivially_constructible = __is_trivially_constructible(T, X...);

namespace impl
{
template<bool  X, class T = void>
struct $When;

template<class T>
struct $When<true, T>
{
    using type = T;
};
}

template<bool X, class T = void>
using $when = typename impl::$When<X, T>::type;
#pragma endregion

namespace impl 
{
template<bool ...V>         struct All;
template<bool ...V>         struct Any;
template<bool V>            struct All<V>       { static constexpr bool $value = V;                      };
template<bool V>            struct Any<V>       { static constexpr bool $value = V;                      };
template<bool V, bool ...W> struct All<V, W...> { static constexpr bool $value = V && All<W...>::$value; };
template<bool V, bool ...W> struct Any<V, W...> { static constexpr bool $value = V || Any<W...>::$value; };
}

template<bool ...V> constexpr bool $all = impl::All<V...>::$value;
template<bool ...V> constexpr bool $any = impl::Any<V...>::$value;

template<class T, class U=void>
struct Is
{
    static constexpr bool $value = $is_base_of<T,U>;
};

template<class T>
struct Is<T, T>
{
    static constexpr bool $value = true;
};

/* test if T is U */
template<class T, class U>
constexpr bool $is = Is<T, U>::$value;

/* test if T is U */
template<class T, class U>
constexpr bool is() {
    return Is<T, U>::$value;
}

/* compile when U==T */
template<class T, class U>
using $when_is = $when<Is<T, U>::$value>;

template<class T, class ...U> constexpr bool $all_is    = $all<is<T,U>()...>; // check if all U is T
template<class T, class ...U> constexpr bool $any_is    = $any<is<T,U>()...>; // check if any U is T

struct $pod;        // check if type is POD
struct $copy;       // check if type is copyable
struct $move;       // check if type is moveable
struct $sint;       // check if type is signed integer
struct $uint;       // check if type is unsigned integer
struct $int;        // check if type is integer
struct $float;      // check if type is floating point
struct $number;     // check if type is number(integer or float)
template<class T>   struct Is<$pod,     T> { static constexpr auto $value = $is_pod<T>; };
template<class T>   struct Is<$sint,    T> { static constexpr auto $value = $is<i8,T>       || $is<i16,T>   || $is<i32,T>   || $is<i64,T>; };
template<class T>   struct Is<$uint,    T> { static constexpr auto $value = $is<u8,T>       || $is<u16,T>   || $is<u32,T>   || $is<u64,T>; };
template<class T>   struct Is<$int,     T> { static constexpr auto $value = $is<$sint,T>    || $is<$uint,T>;    };
template<class T>   struct Is<$float,   T> { static constexpr auto $value = $is<f32,T>      || $is<f64,T>;      };
template<class T>   struct Is<$number,  T> { static constexpr auto $value = $is<$int,T>     || $is<$float,T>;   };
#pragma endregion

}