#pragma once

#ifndef _USTD_MODULE
#ifdef _MSC_VER
#   define _USTD_MODULE 1
#else
#   define _USTD_MODULE 0
#endif
#endif

/* ustd: c++->rust */
#define let const auto
#define mut auto
#define fn  auto
#define use using
#define pub

#define _ustd_cat(a, b)         _ustd_cat_1((a, b))
#define _ustd_cat_1(...)        _ustd_cat_2 __VA_ARGS__
#define _ustd_cat_2(a, b)       a##b

#define unittest(...)           _ustd_test_1((__LINE__, #__VA_ARGS__))
#define _ustd_test_1(...)       _ustd_test_2 __VA_ARGS__

#define _ustd_test_2(id, name)                                                              \
[]] void _ustd_test_func_##id();                                                            \
let _ustd_test_init_##id = ustd::test::install<struct _, id> (name, &_ustd_test_func_##id); \
void _ustd_test_func_##id() [[]


/* ustd: headers */
#if _USTD_MODULE==0
#include <ustd/core.h>
#include <ustd/boxed.h>
#include <ustd/fs.h>
#include <ustd/io.h>
#include <ustd/math.h>
#include <ustd/path.h>
#include <ustd/sync.h>
#include <ustd/test.h>
#include <ustd/thread.h>
#include <ustd/time.h>
#endif

/* ustd: module */
#if _USTD_MODULE==1
import ustd;
#endif


