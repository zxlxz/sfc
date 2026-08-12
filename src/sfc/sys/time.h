#pragma once

#include "sfc/sys/mod.h"

#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/posix/time.h"
#endif

#if defined(_WIN32)
#include "sfc/sys/windows/time.h"
#endif
