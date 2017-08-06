#include <nms/core/property.h>
#include <nms/serialization.h>
#include <nms/io/console.h>
#include <nms/test.h>

namespace nms
{

#pragma region unittest

nms_test(property) {
    using namespace serialization;

    struct TestClass
        : IFormatable
        , ISerializable
    {
        NMS_PROPERTY_BEGIN;
        typedef int NMS_PROPERTY(x) = 1;
        typedef int NMS_PROPERTY(y) = 2;
        typedef int NMS_PROPERTY(z) = 3;
        NMS_PROPERTY_END;
    };

    TestClass obj;
    io::console::writeln("  obj = {");
    io::console::writeln("    {}: {} = {}", 0, obj[$0].name, obj[$0].value);
    io::console::writeln("    {}: {} = {}", 1, obj[$1].name, obj[$1].value);
    io::console::writeln("    {}: {} = {}", 2, obj[$2].name, obj[$2].value);
    io::console::writeln("  }");
}

#pragma endregion

}
