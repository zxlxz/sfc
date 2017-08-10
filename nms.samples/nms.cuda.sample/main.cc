#include <nms/cuda.h>
#include "vadd.cuda.h"

using namespace nms;

int main() {
    cuda::Array<f32> a({ 1024 });
    cuda::Array<f32> b({ 1024 });
    cuda::Array<f32> y({ 1024 });

    cuda::invoke<vadd>(y, a, b);
}
