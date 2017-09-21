#include <nms/test.h>

namespace nms::core
{
#pragma region unittest

nms_test(list) {
    /* copy able */
    List<u32, 32> list;
    for (u32 i = 0; i < 10; ++i) {
        list.append(i);
    }
    io::log::info("list = {}", list);
}

#pragma endregion

}