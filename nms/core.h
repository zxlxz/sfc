#ifndef _NMS_CORE_H_
#define _NMS_CORE_H_

#include <nms/config.h>

#if defined(NMS_CC_MSVC) && !defined(NMS_CC_CLANG)
#include <nms/core/cpp.msvc.h>
#else
#include <nms/core/cpp.h>
#endif

#include <nms/core/base.h>
#include <nms/core/trait.h>
#include <nms/core/type.h>
#include <nms/core/vec.h>
#include <nms/core/view.h>
#include <nms/core/memory.h>
#include <nms/core/string.h>
#include <nms/core/format.h>
#include <nms/core/parse.h>

#include <nms/core/time.h>
#include <nms/core/exception.h>
#include <nms/core/view.h>
#include <nms/core/list.h>
#include <nms/core/time.h>

#endif
