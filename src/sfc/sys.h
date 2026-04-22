#pragma once

#include "sfc/sys/raw.h"

namespace sfc::sys {

using raw::RawFd;
using raw::ErrCode;

#ifndef _SFC_SYS_IO_
using raw::File;
#endif

#ifndef _SFC_SYS_THREAD_
using raw::Thread;
#endif

#ifndef _SFC_SYS_SYNC_
using raw::Condvar;
using raw::Mutex;
#endif

}  // namespace sfc::sys
