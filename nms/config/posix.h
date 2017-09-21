#pragma once

/*!
 * posix headers: http://pubs.opengroup.org/onlinepubs/9699919799/idx/head.html
 */

#ifdef NMS_BUILD

#ifdef NMS_OS_UNIX
#include <unistd.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <execinfo.h>
#include <sys/ioctl.h>
#endif

#ifdef NMS_OS_WINDOWS
#include <io.h>
#include <direct.h>
#include <process.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>

#ifdef NMS_OS_WINDOWS
using stat_t = struct _stat64;
static __forceinline int fstat(int fd, stat_t* st) {
    return ::_fstat64(fd, st);
}

static __forceinline int stat(const char* path, stat_t* st) {
    return ::_stat64(path, st);
}
#else
using stat_t = struct stat;
#endif

#endif
