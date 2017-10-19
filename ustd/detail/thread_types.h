#pragma once

#define _THREAD_TYPES

namespace ustd
{

using  thrd_t = struct thrd_st*;

#if   defined(_WIN32)
typedef union { char data[48];  long long align; } mtx_t;
typedef struct _cnd_st*  cnd_t;
#elif defined(__linux)
typedef union { char data[40];  long long align; } mtx_t;
typedef union { char data[48];  long long align; } cnd_t;
#elif defined(__APPLE__)
typedef union { char data[64];  long long align; } mtx_t;
typedef union { char data[00];  long long align; } cnd_t;
#endif

}
