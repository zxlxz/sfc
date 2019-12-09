#ifndef rc_INL
#define rc_INL

#ifdef _MSC_VER
#include <corecrt.h>
#include <string.h>
#include <stdio.h>
#endif

#ifdef __clang__
#pragma clang diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif

#ifdef _WIN32
#define pub __declspec(dllexport)
#else
#define pub __attribute__((visibility("default")))
#endif

#endif
