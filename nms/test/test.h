#pragma once

#include <nms/core.h>
#include <nms/io/log.h>
#include <nms/io/console.h>

namespace nms::test
{

NMS_API u32  install(StrView name, void (*func)());
NMS_API u32  invoke(const View<StrView>& masks);

}

#define nms_test(func, ...)                                                                                 \
    static void func##_test();                                                                              \
    static auto func##_temp   = nms::test::install(nms::typeof<struct func##_tag>().name(), func##_test);   \
    static void func##_test()
