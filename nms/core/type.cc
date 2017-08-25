#include <nms/core.h>
#include <nms/serialization.h>
#include <nms/test.h>

namespace nms
{

#pragma region unittest

NMS_ENUM(TestEnum, E0, E1, E2);

struct TestClass
    : IFormatable
    , serialization::ISerializable
{
    NMS_PROPERTY_BEGIN;
    typedef TestEnum    NMS_PROPERTY(e) = TestEnum::E0;
    typedef int         NMS_PROPERTY(x) = 1;
    typedef int         NMS_PROPERTY(y) = 2;
    typedef int         NMS_PROPERTY(z) = 3;
    NMS_PROPERTY_END;
};

nms_test(property) {
    TestClass obj;
    io::log::info("obj = {}", obj);
}

#pragma endregion

}
