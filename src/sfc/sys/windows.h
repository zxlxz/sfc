#pragma once

#include "sfc/sys/windows/io.h"
#include "sfc/sys/windows/sync.h"
#include "sfc/sys/windows/thread.h"

#ifdef _SFC_SYS_ALLOC_
#include "sfc/sys/windows/alloc.h"
#endif

#ifdef _SFC_SYS_ENV_
#include "sfc/sys/windows/env.h"
#endif

#ifdef _SFC_SYS_BACKTRACE_
#include "sfc/sys/windows/backtrace.h"
#endif

#ifdef _SFC_SYS_TIME_
#include "sfc/sys/windows/time.h"
#endif

#ifdef _SFC_SYS_FS_
#include "sfc/sys/windows/fs.h"
#endif

#ifdef _SFC_SYS_LIBRARY_
#include "sfc/sys/windows/library.h"
#endif

namespace sfc::sys {
using namespace windows;
}
