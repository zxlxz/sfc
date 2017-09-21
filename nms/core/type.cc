#include <nms/core.h>
#include <nms/test.h>

namespace nms::core
{

#pragma region unittest

class TestClass
{ };

nms_test(property) {
    io::log::info("TestClass.name = {}", typeof<TestClass>().name());
}

#pragma endregion

}
