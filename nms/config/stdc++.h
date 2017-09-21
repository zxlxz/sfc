#pragma once

/*!
 * cppreference: http://en.cppreference.com/w/cpp
 */

#ifdef NMS_BUILD

/* check: c++ */
#ifndef __cplusplus
#   error "nmscc should compile with c++"
#endif

/* check: c++14 */
#ifdef NMS_CC_MSVC
#   if _MSVC_LANG < 201402L
#       error "nmscc shold compile with c++14"
#   endif
#else
#   if __cplusplus < 201402L
#       error "nmscc shold compile with c++14"
#   endif
#endif

#ifdef NMS_CC_GNUC
#include <cxxabi.h>
#endif

#include <typeinfo>

#endif

#include <new>

#if defined(_TYPEINFO_) || defined(__LIBCPP_TYPEINFO) || defined(_TYPEINFO)
#   define NMS_STDCXX_TYPEINFO      // check: has include <typeinfo>
#endif

#if defined(_UTILITY_) || defined(_LIBCPP_UTILITY) || defined(_UTILITY)
#   define NMS_STDCXX_UTILITY       // check: has include <utility>
#endif
