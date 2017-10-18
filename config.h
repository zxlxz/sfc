#ifndef _USTD_POSIX_H_
#define _USTD_POSIX_H_

#if     defined(_WIN32)
#include "sys/win32.h"
#elif   defined(__linux)
#include "sys/linux.h"
#elif   defined(__APPLE__)
#include "sys/apple.h"
#endif

#undef stdin    // 0
#undef stdout   // 1
#undef stderr   // 2

#define _USTD_IMPL  1
#endif
