#ifdef _SFC_SYS_ALLOC_
#include "sfc/sys/posix/alloc.h"
#endif

#ifdef _SFC_SYS_ENV_
#include "sfc/sys/posix/env.h"
#endif

#ifdef _SFC_SYS_FS_
#include "sfc/sys/posix/fs.h"
#endif

#ifdef _SFC_SYS_FFI_
#include "sfc/sys/posix/ffi.h"
#endif

#ifdef _SFC_SYS_IO_
#include "sfc/sys/posix/io.h"
#endif

#ifdef _SFC_SYS_SYNC_
#include "sfc/sys/posix/sync.h"
#endif

#ifdef _SFC_SYS_THREAD_
#include "sfc/sys/posix/thread.h"
#endif

#ifdef _SFC_SYS_TIME_
#include "sfc/sys/posix/time.h"
#endif

#ifdef _SFC_SYS_BACKTRACE_
#include "sfc/sys/posix/backtrace.h"
#endif

namespace sfc::sys {

namespace posix {};

#if defined(__unix__) || defined(__APPLE__)
using namespace posix;
#endif

}  // namespace sfc::sys
