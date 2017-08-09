#ifndef _NMS_INL_H_
#define _NMS_INL_H_

#ifdef _MSC_VER
#   define _CRT_NONSTDC_NO_WARNINGS     1
#   define _CRT_SECURE_NO_WARNINGS      1
#   define _CRT_OBSOLETE_NO_WARNINGS    1
#endif


#include <stdio.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
#   include <unistd.h>
#endif

#ifdef _MSC_VER
#   include <io.h>
#   include <direct.h>
#   include <process.h>
#endif

#define NMS_BUILD

#endif
