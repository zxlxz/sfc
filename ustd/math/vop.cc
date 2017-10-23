#include "config.h"

namespace ustd::math::vop
{

[unittest(assign)] {
    mut a = Array<f32,2>({8u, 8u});
    a <<= vline(1.0f, 0.1f);
    println("a = {}", a);
}

[unittest(foreach)] {
    let a = Array<f32,2>({8u, 8u});
    a <<= vline(1.0f, 0.1f);
    a <<= vsin(a);
    println("a = { }", a);

    let b = Array<f32, 2>({ 8u, 8u });
    b <<= a + vsin(a)*vcos(a);

    println("b = { }", b);
}


}

