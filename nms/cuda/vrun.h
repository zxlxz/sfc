
#include <nms/cuda/base.h>
#include <nms/cuda/runtime.h>

namespace nms::cuda
{

class Program;
class Module;

struct Vrun
{
public:
    /* foreach */
    template<class Tfunc, class Tret, class Targ>
    static void foreach(Tfunc func, Tret& ret, const Targ& arg) {
        _foreach(func, ret, arg, Tver<1>{});
    }

    /* cuda: signin cuda kernel */
    template<class Tfunc, class Tret, class Targ>
    static u32 _signin() {
        static const auto fid = _signin_impl(typeof<Tfunc>().name(), typeof<Tret>().name(), typeof<Targ>().name());
        return fid;
    }

protected:
    NMS_API static Program& sProgram();
    NMS_API static Module&  sModule();

private:
    /* foreach: redirect to cuda::mcpy */
    template<class T, u32 N>
    static void _foreach(Ass2 /*func*/, const nms::View<T, N>& dst, const nms::View<T, N>& src, Tver<1>) {
        const auto dst_ptr = const_cast<T*>(dst.data());
        const auto src_ptr = src.data();
        const auto cpy_len = dst.count();
        cuda::mcpy(dst_ptr, src_ptr, cpy_len);
    }

    /* foreach: redirect to cuda::invoke */
    template<class Tfunc, class Tret, class Targ>
    static void _foreach(Tfunc /*func*/, const Tret& ret, const Targ& arg, Tver<0>) {
        static const auto fid  = nms::static_init<Tfunc(Tret, Targ), &_signin<Tfunc, Tret, Targ> >();
        static const auto kid  = _get_kernel(fid);
        static const auto&mod  = sModule();

        const  auto dims  = ret.size();
        mod.invoke(kid, dims.$count, dims.data, ret, arg);
    }

    /* get kernel */
    NMS_API static Module::fun_t _get_kernel(u32 fid);

    /* signin: impl */
    NMS_API static u32 _signin_impl(StrView func, StrView ret_type, StrView arg_type);
};

inline cuda::Vrun operator||(const cuda::Vrun&, const cuda::Vrun&) {
    return {};
}

inline cuda::Vrun operator||(const math::Vrun&, const cuda::Vrun&) {
    return {};
}

inline cuda::Vrun operator||(const cuda::Vrun&, const math::Vrun&) {
    return {};
}

}
