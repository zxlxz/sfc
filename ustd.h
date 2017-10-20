#pragma once

#ifndef _USTD_MODULE
#   define _USTD_MODULE 1
#endif

/* ustd: c++->rust */
#define let const auto
#define mut auto
#define fn  auto
#define use using

#define _ustd_test_var(...)  _ustd_test_var1 __VA_ARGS__
#define _ustd_test_var1(id)  _ustd_test_##id
#define unittest(...)        []] int _ustd_test_var((__LINE__)) = ::ustd::test::scheduler().install<struct _>(#__VA_ARGS__) << [

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


