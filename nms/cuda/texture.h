#pragma once

#include <nms/cuda/runtime.h>
#include <nms/cuda/array.h>

namespace nms::cuda
{

template<class T, u32 N>
struct TexView
{
    using u32xN = Vec<u32, N>;
    using BorderMode = TexAddressMode;
    using FilterMode = TexFilterMode;

protected:
    u64 obj_;   // cuda texture object

    TexView()
        : obj_(0)
    {}
};

template<class T, u32 N>
class Texture
    : public TexView<T,N>
    , public INocopyable
{
    using base = TexView<T, N>;

public:
    using base::u32xN;
    using base::BorderMode;
    using base::FilterMode;

    Texture(const u32(&dim)[N], TexAddressMode border_mode, TexFilterMode filter_mode =FilterMode::Point)
        : base(), arr_(nullptr), dim_(dim)
    {
        arr_        = arr_new<T>(dim);
        base::obj_  = tex_new(arr_, border_mode, filter_mode);
    }

    ~Texture() {
        if (base::obj_ != 0) {
            tex_del(base::obj_);
        }
        if (arr_ != nullptr) {
            arr_del(arr_);
        }
    }

private:
    arr_t   arr_;   // cuda array
    u32xN   dim_;   // texture dim
};

}


namespace nms::math
{

template<class T, u32 N>
cuda::TexView<T, N> toLambda(cuda::TexView<T, N> v) {
    return v;
}

}
