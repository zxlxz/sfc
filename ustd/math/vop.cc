#include "config.h"

namespace ustd::math
{

using namespace vop;

[unittest(vop::assign)] {
    mut a = Array<f32,2>({8u, 8u});
    a <<= vline(1.0f, 0.1f);
    println("a = {}", a);
};

}
