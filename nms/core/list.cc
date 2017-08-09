#include <nms/test.h>
#include <nms/core/list.h>
#include <nms/io/console.h>

namespace nms
{
#pragma region unittest

static void uncopyable_list_test() {
    class Uncopyable
    {
    public:
        Uncopyable(int)
        {}
    };

    List<Uncopyable> list;
    Uncopyable x(0);
    list.append(move(x));
}

nms_test(list) {
    /* copy able */
    List<u32> list;
    for (u32 i = 0; i < 100; ++i) {
        list.append(i);
    }
    io::console::writeln("list = {}", list);


}

#pragma endregion

}