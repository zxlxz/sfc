#pragma once

#include <ustd/os/default.h>

namespace ustd::os
{

using  thrd_t = struct thrd_st*;

namespace windows
{
typedef union { char data[48];  long long align; } mtx_t;
typedef struct _cnd_st*  cnd_t;
}

namespace linux
{
typedef union { char data[40];  long long align; } mtx_t;
typedef union { char data[48];  long long align; } cnd_t;
}

namespace apple
{
typedef union { char data[64];  long long align; } mtx_t;
typedef union { char data[64];  long long align; } cnd_t;
}

}
