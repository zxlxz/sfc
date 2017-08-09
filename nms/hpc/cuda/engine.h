#pragma once

#include <nms/core.h>
#include <nms/math.h>
#include <nms/hpc/cuda/runtime.h>

struct _nvrtcProgram;

namespace nms::hpc::cuda
{

template<class T, u32 N>
struct View;

class Program;
class Module;

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

    template<class F, class ...T>
    void addfunc() {
        const auto      func = typeof<F>().name();
        const StrView   args[] = { typeof<T>().name()... };
        addfunc(func, mkView(args));
    }

    StrView src() const {
        return src_;
    }

    StrView ptx() const {
        return ptx_;
    }

protected:
    String  src_;
    String  ptx_;
    u32     cnt_;

    NMS_API u32 add_foreach(StrView func_type, StrView ret_type, StrView arg_type);
};

NMS_API Program& gProgram();

/**
 * cuda-foreach-executor
 */
struct ForeachExecutor
{
public:
    template<class Tfunc, class Tret, class ...Targs>
    static void run(Tfunc func, Tret& ret, const Targs& ...args) {
        cuda_exec(func, ret, args...);
    }

    template<class Tret, class ...Targs>
    struct ID;

    /* cuda-exec: try copy, than invoke */
    template<class T, u32 N>
    static void cuda_exec(Ass2 func, nms::View<T,N>& dst, const nms::View<T,N>& src) {
        if (dst.isNormal() && src.isNormal()) {
            auto count = dst.numel();
            cuda::mcpy(dst.data(), src.data(), count);
        }
        else {
            cuda_foreach(func, dst, src);
        }
    }

    /* cuda-exec: redirect to invoke */
    template<class Tfunc, class Tret, class ...Targs>
    static void cuda_exec(Tfunc func, Tret& ret, const Targs& ...args) {
        cuda_foreach(func, ret, args...);
    }

    /* cuda-invoke */
    template<class Tfunc, class Tret, class Targ>
    static void cuda_foreach(Tfunc func, Tret& ret, const Targ& arg) {
        static auto  fid = static_run< &cuda_foreach_id<Tfunc, Tret, Targ>, ID<Tret, Targ> >();
        gModule().invoke(fid, ret, arg);
    }

    template<class Tfunc, class Tret, class Targ>
    static u32 cuda_foreach_id() {
        static auto&   program = gProgram();
        static auto    func_id = program.add_foreach(typeof<Tfunc>().name(), typeof<Tret>().name(), typeof<Targ>().name());
        return func_id;
    }
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

}
