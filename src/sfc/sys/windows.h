#ifdef _SFC_SYS_ALLOC_
#include "sfc/sys/windows/alloc.h"
#endif

#ifdef _SFC_SYS_ENV_
#include "sfc/sys/windows/env.h"
#endif

#ifdef _SFC_SYS_FS_
#include "sfc/sys/windows/fs.h"
#endif

#ifdef _SFC_SYS_FFI_
#include "sfc/sys/windows/ffi.h"
#endif

#ifdef _SFC_SYS_IO_
#include "sfc/sys/windows/io.h"
#endif

#ifdef _SFC_SYS_SYNC_
#include "sfc/sys/windows/sync.h"
#endif

#ifdef _SFC_SYS_THREAD_
#include "sfc/sys/windows/thread.h"
#endif

#ifdef _SFC_SYS_TIME_
#include "sfc/sys/windows/time.h"
#endif

#ifdef _SFC_SYS_BACKTRACE_
#include "sfc/sys/windows/backtrace.h"
#endif

namespace sfc::sys {

namespace windows {};

#ifdef _WIN32
using namespace windows;
#endif
}  // namespace sfc::sys
