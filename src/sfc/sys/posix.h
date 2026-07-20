#pragma once

#include "sfc/sys/posix/io.h"
#include "sfc/sys/posix/sync.h"
#include "sfc/sys/posix/thread.h"

#ifdef _SFC_SYS_ALLOC_
#include "sfc/sys/posix/alloc.h"
#endif

#ifdef _SFC_SYS_ENV_
#include "sfc/sys/posix/env.h"
#endif

#ifdef _SFC_SYS_BACKTRACE_
#include "sfc/sys/posix/backtrace.h"
#endif

#ifdef _SFC_SYS_TIME_
#include "sfc/sys/posix/time.h"
#endif

#ifdef _SFC_SYS_FS_
#include "sfc/sys/posix/fs.h"
#endif

#ifdef _SFC_SYS_FFI_
#include "sfc/sys/posix/ffi.h"
#endif

namespace sfc::sys {
using namespace posix;
}
