#pragma once

#include <nms/core.h>

namespace nms::test
{

NMS_API u32  install(StrView name, void (*func)());
NMS_API u32  invoke(const StrView name[], u32 count);

}

#define nms_test(func, ...)                                                         \
    static void func##_test();                                                      \
    static auto func##_func() { return nms::StrView{ __PRETTY_FUNCTION__}; }        \
    static auto func##_temp   = nms::test::install(func##_func(), func##_test);     \
    static void func##_test()
