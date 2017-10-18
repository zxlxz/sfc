#pragma once

#include <ustd/core.h>
#include <ustd/time.h>
#include <ustd/io.h>
#include <ustd/boxed.h>

namespace ustd::thread
{

#ifndef _THREADS_H    
using  thrd_t = struct thrd_st*;
#endif

template<class T>
using Result = result::Result<T, int>;

using ThreadId = int;

struct Thread
{
    struct Builder;

    template<class T>
    class JoinHandle;

    thrd_t _handle;

    fn id() const -> ThreadId;

  private:
    Thread() = default;
    fn _detach() -> void;
};

template<class T=void>
using JoinHandle = Thread::JoinHandle<T>;

template<>
class Thread::JoinHandle<void>
{
    friend struct Thread::Builder;
    Thread  _thr;

  public:
    JoinHandle(JoinHandle&& other) {
        _thr._handle = other._thr._handle;
        other._thr._handle = nullptr;
    }

    JoinHandle(const JoinHandle&)   = delete;
    fn operator=(const JoinHandle&) = delete;

    ~JoinHandle() {
        _thr._detach();
    }

    fn thread() -> Thread& {
        return _thr;
    }

   fn join()&& -> Result<void>;

  private:
    JoinHandle(Thread thr): _thr{ thr } {}
};

struct Thread::Builder
{
    template<class F, typename T=decltype(declval<F>()())>
    fn spawn(F&& f) -> Result<JoinHandle<void>> {
        return _spawn($type<T>{}, as_fwd<F>(f));
    }

  private:
    template<typename F>
    static fn _spawn($type<void>, F&& f) -> Result<JoinHandle<void>> {
        using Result = Result<JoinHandle<void>>;
        using Box    = boxed::Box<val_t<F>>;

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
            return Result::Ok(JoinHandle<void>{thr.ok().unwrap()});
        }
        else {
            return Result::Err(thr.err().unwrap());
        }
    }

   static fn _spawn_impl(void* obj, int(*func)(void*)) -> Result<Thread>;
};

template<typename F, typename T = decltype((declval<F>())()) >
fn spawn(F&& f) -> JoinHandle<T> {
    return Thread::Builder().spawn(as_fwd<F>(f)).unwrap();
}

fn sleep(time::Duration dur) -> void;
fn sleep_ms(u32 ms) -> void;

}
