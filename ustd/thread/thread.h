#pragma once

#include <ustd/boxed.h>
#include <ustd/detail/thread_types.h>

namespace ustd::thread
{

template<class T>
using Result = result::Result<T, int>;

struct Thread;

struct Builder;

template<class T>
class JoinHandle;

struct Thread
{
    thrd_t _handle;

    fn id() const -> int;

    static fn current() -> Thread;

private:
    friend struct Builder;
    template<class T> friend class JoinHandle;

    Thread(thrd_t handle) : _handle(handle) {}

    fn _detach() -> void;
};

template<>
class JoinHandle<void>
{
public:
    JoinHandle(Thread thr) : _thr{ thr } {}

    JoinHandle(JoinHandle&& other) : _thr{ other._thr._handle } {
        other._thr._handle = nullptr;
    }

    ~JoinHandle() {
        _thr._detach();
    }

    fn thread() -> Thread& {
        return _thr;
    }

    fn join() && ->Result<void>;

private:
    Thread  _thr;
    friend struct Builder;

    JoinHandle(const JoinHandle&) = delete;
    fn operator=(const JoinHandle&) = delete;
};

struct Builder
{
    template<class F, typename T = decltype(declval<F>()())>
    fn spawn(F&& f) -> Result<JoinHandle<void>> {
        return _spawn($type<T>{}, as_fwd<F>(f));
    }

private:
    template<typename F>
    static fn _spawn($type<void>, F&& f) -> Result<JoinHandle<void>> {
        using Result = Result<JoinHandle<void>>;
        using Box = boxed::Box<val_t<F>>;

        let box = Box(as_fwd<F>(f));
        let thr = _spawn_impl((&*box), [](void* args) -> int {
            let box = Box::from_raw(reinterpret_cast<F*>(args));
            try {
                (*box)();
            }
            catch (...) {
                return -1;
            }
            return 0;
        });
        box.forget();

        if (thr.is_ok()) {
            mut handle = JoinHandle<void>(thr.ok().unwrap());
            return Result::Ok(as_mov(handle));
        }
        else {
            return Result::Err(thr.err().unwrap());
        }
    }

    static fn _spawn_impl(void* obj, int(*func)(void*))->Result<Thread>;
};

template<typename F, typename T = decltype((declval<F>())()) >
fn spawn(F&& f) -> JoinHandle<T> {
    return Builder().spawn(as_fwd<F>(f)).unwrap();
}

inline fn current() -> Thread {
    return Thread::current();
}

}
