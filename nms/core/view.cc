#include <nms/core.h>
#include <nms/test.h>

namespace nms
{

NMS_API void ViewInfo::format(IString& buf) const {
    sformat(buf, "{:c}{}x{}", type, size * 8, rank);
}

template<class Tview>
static void assert_dims(const Tview& view, const typename Tview::Tdims& dims) {
    auto view_dims = view.size();
    test::assert_eq(view_dims, dims);
}

nms_test(view_slice) {

    View<f32, 4> v8888(nullptr, { 8, 8, 8, 8 });

    assert_dims(v8888, { 8u, 8u, 8u, 8u });
    assert_dims(v8888.slice({ 0, -1 }, { 0, -1 }, { 0, -1 }, { 0, -1 }), { 8u, 8u, 8u, 8u });
    assert_dims(v8888.slice({ 0, 1 }, { 0, 1 }, { 0, 1 }, { 0, 1 }), { 2u, 2u, 2u, 2u });

    assert_dims(v8888.slice({ 0 }, { 0, 1 }, { 0, 1 }, { 0, 1 }), { 2u, 2u, 2u });
    assert_dims(v8888.slice({ 0, 1 }, { 0 }, { 0, 1 }, { 0, 1 }), { 2u, 2u, 2u });
    assert_dims(v8888.slice({ 0, 1 }, { 0, 1 }, { 0 }, { 0, 1 }), { 2u, 2u, 2u });
    assert_dims(v8888.slice({ 0, 1 }, { 0, 1 }, { 0,1 }, { 0  }), { 2u, 2u, 2u });

    assert_dims(v8888.slice({ 0 }, { 0 }, { 0 }, { 0, -1 }), { 8u });
    assert_dims(v8888.slice({ 0 }, { 0 }, { 0, -1 }, { 0 }), { 8u });
    assert_dims(v8888.slice({ 0 }, { 0, -1 }, { 0 }, { 0 }), { 8u });
    assert_dims(v8888.slice({ 0, -1 }, { 0 }, { 0 }, { 0 }), { 8u });


    View<f32, 4> v1111(nullptr, { 1, 1, 1, 1 });
    assert_dims(v1111, { 1u, 1u, 1u, 1u });
    assert_dims(v1111.slice({ 0, -1 }, { 0, -1 }, { 0, -1 }, { 0, -1 }), { 1u, 1u, 1u, 1u });
}

}
