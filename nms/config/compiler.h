#pragma once

#ifdef NMS_CC_MSVC
#pragma warning(disable: 4127)      // conditional expression is constant
#pragma warning(disable: 4505)      // unreferenced inline function has been removed
#pragma warning(disable: 4514)      // unreferenced inline function has been removed
#pragma warning(disable: 4710)      // function not inlined
#pragma warning(disable: 4711)      // function performed inlined
#pragma warning(disable: 4820)      // n bytes padding add after data member ...
#pragma warning(disable: 6326)      // potential comparison of a constant with another constant

#ifdef NMS_BUILD
// warning: disable microsoft unstandard warnings.
#define _CRT_NONSTDC_NO_WARNINGS     1
#define _CRT_SECURE_NO_WARNINGS      1
#define _CRT_OBSOLETE_NO_DEPRECATE   1
#endif

#endif

/* define: __forceinline */
#ifdef NMS_CC_GNUC
#   define __forceinline inline __attribute__((always_inline))
#endif


/* define: __PRETTY_FUNCTION__ */
#ifdef NMS_CC_MSVC
#   define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

/* define: __noop */
#ifndef NMS_CC_MSVC
#   define __noop(...)
#endif

/* define: NMS_CC_INTELLISENSE */
#ifdef NMS_CC_MSVC
#ifdef __INTELLISENSE__
#   define NMS_CC_INTELLISENSE
#   pragma warning(disable: 4100)
#endif
#endif

/* define: NMS_API */
#ifndef NMS_API
#if defined(NMS_CC_MSVC)
#if defined NMS_BUILD
#   define NMS_API __declspec(dllexport)
#else
#   define NMS_API __declspec(dllimport)
#endif
#elif defined(NMS_CC_CLANG)
#   define NMS_API __attribute__((visibility("default")))
#else
#   define NMS_API
#endif
#endif

/* define: NMS_ABI */
#ifndef NMS_ABI
#   define NMS_ABI extern "C" NMS_API
#endif

