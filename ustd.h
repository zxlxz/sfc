#pragma once

#ifndef __INTELLISENSE__
#   define _USED_MODULE_
#endif

#ifdef _USTD_MODULE_
module ustd;
#define let const auto
#define mut auto
#define fn  auto
#endif

#ifndef _USTD_MODULE_
#include <ustd/core.h>
#include <ustd/boxed.h>
#include <ustd/fs.h>
#include <ustd/io.h>
#include <ustd/path.h>
#include <ustd/sync.h>
#include <ustd/test.h>
#include <ustd/thread.h>
#include <ustd/time.h>
#endif
