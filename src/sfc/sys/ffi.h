#pragma once

#include "sfc/sys/mod.h"

#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/posix/ffi.h"
#endif

#if defined(_WIN32)
#include "sfc/sys/windows/ffi.h"
#endif
