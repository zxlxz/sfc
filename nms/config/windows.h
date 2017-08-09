#pragma once

#define _CRT_NONSTDC_NO_WARNINGS     1
#define _CRT_SECURE_NO_WARNINGS      1
#define _CRT_OBSOLETE_NO_DEPRECATE   1

// stdc
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

// posix
#include <fcntl.h>
#include <io.h>
#include <direct.h>
#include <process.h>

// system
#include <sys/stat.h>

#define NMS_API __declspec(dllexport)

