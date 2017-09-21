#ifndef _NMS_CONFIG_H_
#define _NMS_CONFIG_H_

/* check system */
#if defined(_WIN32)
#   define NMS_OS_WINDOWS                   // check if os == windows
#elif defined(__unix__) || defined(__APPLE__)
#   define NMS_OS_UNIX                      // check if os == unix
#else
#   error "unknow system"
#endif

#ifdef __APPLE__
#   define NMS_OS_APPLE                     // check if os == macos
#endif

#ifdef __linux
#   define NMS_OS_LINUX                     // check if os == linux
#endif

/* check compilier */
#if _MSC_VER
#   define NMS_CC_MSVC                      // check if compiler == msvc
#endif

#ifdef __clang__
#   define NMS_CC_CLANG                     // check if compiler == clang
#endif

#ifdef __GNUC__
#   define NMS_CC_GNUC                      // check if compiler == gcc
#endif


#include <nms/config/compiler.h>
#include <nms/config/stdc.h>
#include <nms/config/posix.h>
#include <nms/config/stdc++.h>

#endif  // _NMS_CONFIG_H_
