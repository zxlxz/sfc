
#ifndef _NMS_CONFIG_H_
#define _NMS_CONFIG_H_

/* check Operating System */
#if defined(_WIN32)
#   define NMS_OS_WINDOWS           // check if os == windows
#   include <nms/config/windows.h>
#elif defined(__APPLE__)
#   define NMS_OS_APPLE             // check if os == macos
#   define NMS_OS_UNIX              // check if os == macos
#   include <nms/config/apple.h>
#elif defined(__unix__)
#   define NMS_OS_UNIX              // check if os == linux
#   include <nms/config/unix.h>
#endif

/* check compilier */
#if defined(_MSC_VER)
#   define NMS_CC_MSVC              // check if compiler == msvc
#   define __PRETTY_FUNCTION__ __FUNCSIG__
#   pragma warning(disable:6326)    // E6326: potential comparison of a constant with another constant
#elif defined(__GNUC__)
#   define NMS_CC_GNUC              // check if compiler == gcc
#   define __forceinline inline __attribute__((always_inline))
#elif defined(__clang__)
#   define NMS_CC_CLANG             // check if compiler == clang
#   define __forceinline inline __attribute__((always_inline))
#endif

#ifndef NMS_API
#if defined(NMS_CC_MSVC)
#   define NMS_API __declspec(dllimport)
#elif defined(NMS_CC_CLANG)
#   define NMS_API __attribute__((visibility("default")))
#else
#   define NMS_API
#endif
#endif

#ifndef NMS_ABI
#   define NMS_ABI extern "C" NMS_API
#endif
#pragma endregion

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include <new>

#endif  // _NMS_CONFIG_H_
