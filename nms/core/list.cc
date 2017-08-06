#include <nms/test.h>
#include <nms/core/list.h>
#include <nms/io/console.h>

namespace nms
{
#pragma region unittest
nms_test(list) {

    List<u32> list;
    for (u32 i = 0; i < 100; ++i) {
        list.append(i);
    }

    io::console::writeln("list = {}", list);
}

#pragma endregion

}