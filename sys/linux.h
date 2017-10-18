#pragma once

#ifdef __linux


/* stdc */
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include "posix/threads.h"

/* posix */
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <dlfcn.h>

/* sys */
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#endif
