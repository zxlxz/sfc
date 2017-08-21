#pragma once

#include <nms/cuda/base.h>
#include <nms/cuda/runtime.h>

struct _nvrtcProgram;

namespace nms::cuda
{

struct ForeachExecutor;

/**
 * cuda program
 */
class Program : public INocopyable
{
    friend class Module;
    friend struct ForeachExecutor;

public:
    /**
     * constructor
     * @param source: cuda source (*.cu)
     */
    NMS_API Program(StrView source);
    NMS_API ~Program();

    /**
     * complile cuda source to ptx
     */
    NMS_API bool compile();

    /*!
     * add source to the program
     */
    NMS_API void addSrc(StrView src);

    StrView src() const {
        return src_;
    }

    StrView ptx() const {
        return ptx_;
    }

protected:
    String  src_;
    String  ptx_;
};

/**
 * cuda-foreach-executor
 */
struct ForeachExecutor
{
public:
    template<class Tfunc, class Tret, class Targ>
    static void run(Tfunc func, Tret& ret, const Targ& arg) {
        _run(func, ret, arg, Version<1>{});
    }

    /* add func */
    template<class Tfunc, class Tret, class Targ>
    static u32 add_func() {
        static const auto fid = _add_func(typeof<Tfunc>().name(), typeof<Tret>().name(), typeof<Targ>().name());
        return fid;
    }

protected:
    NMS_API static Program& sProgram();
    NMS_API static Module&  sModule();

private:
    /* run: try copy, than invoke */
    template<class T, u32 N>
    static void _run(Ass2 func, const nms::View<T, N>& dst, const nms::View<T, N>& src, Version<1>) {
        if (dst.isNormal() && src.isNormal()) {
            auto count = dst.count();
            cuda::mcpy(const_cast<T*>(dst.data()), src.data(), count);
        }
        else {
            _run(func, dst, src, Version<0>{});
        }
    }

    /* run: redirect to invoke */
    template<class Tfunc, class Tret, class Targ>
    static void _run(Tfunc func, const Tret& ret, const Targ& arg, Version<0>) {
        static auto fid  = nms::static_run< &add_func<Tfunc, Tret, Targ>, Tfunc(Tret, Targ)>();
        static auto kid  = _get_kernel(fid);
        static auto&mod  = sModule();

        const auto dims  = ret.size();
        mod.invoke(kid, dims.$rank, dims.data(), ret, arg);
    }

    /* get kernel */
    NMS_API static Module::fun_t _get_kernel(u32 fid);

    /* add func */
    NMS_API static u32 _add_func(StrView func, StrView ret_type, StrView arg_type);
};

inline ForeachExecutor operator||(const cuda::ForeachExecutor&, const cuda::ForeachExecutor&) {
    return {};
}

inline ForeachExecutor operator||(const math::ForeachExecutor&, const cuda::ForeachExecutor&) {
    return {};
}

inline ForeachExecutor operator||(const cuda::ForeachExecutor&, const math::ForeachExecutor&) {
    return {};
}

namespace engine
{

NMS_API Program& gProgram();
NMS_API Module&  gModule();

inline void add_src(StrView src) {
    gProgram().addSrc(src);
}

template<class Tfunc, Tfunc* func>
struct Kfunc;

template<class ...Targ, void(*func)(Targ...)>
struct Kfunc< void(Targ...), func >
{
    Kfunc(StrView name)
    {
        static auto& mod = gModule();
        kid_ = mod.get_kernel(name);
    }

    template<u32 N>
    struct Runner
    {
    public:
        void operator()(Targ ...args) {
            static auto& mod = gModule();
            mod.invoke(kid_, N, size_.data(), args...);
        }

    protected:
        Module::fun_t   kid_;
        Vec<u32, N>     size_;

    private:
        friend struct Kfunc;
        Runner(Module::fun_t kid, const u32(&dims)[N]) 
            : kid_(kid), size_(dims)
        {}
    };

    template<u32 N>
    Runner<N> operator[](const u32(&dims)[N]) {
        return { kid_, dims };
    }

protected:
    Module::fun_t kid_;
};

template<class Tfunc, Tfunc* func, u32 N, class ...Targ>
void invoke(StrView name, const u32 (&dims)[N], Targ&& ...args) {
    Kfunc<Tfunc, func> kfunc(name);
    kfunc[dims](args...);
}

}

}

#define nms_cuda_kfunc(func)   nms::cuda::engine::Kfunc<decltype(func), &func>{#func}
