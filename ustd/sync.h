#pragma once

#include <ustd/core.h>

namespace ustd::sync
{

union alignas(void*) mtx_t {
#if defined(__linux)
        u8  _data[40];
#elif defined(_WIN32)
        u8  _data[48];
#elif defined(__APPLE__)
        u8  _data[56 + 8];
#endif
};

enum LockError
{
    Success,
    WouldBlock,
    Other
};

template<class T>
using LockResult = result::Result<T, int>;

class Mutex
{
    mtx_t   _mtx;
    i64     _stat;

  public:
    Mutex()  noexcept {
        _init();
    }

    ~Mutex() noexcept {
        if (_stat != 0) {
            return;
        }
        _destroy();
    }

    Mutex(Mutex&& m) noexcept: _mtx(m._mtx) {
        m._stat = -1;
    }

    Mutex(const Mutex&) = delete;
    void operator=(const Mutex&) = delete;

  public:
    class Guard
    {
        friend class Mutex;

        Mutex*  _mutex;

      protected:
        Guard(Mutex& mutex) noexcept
            : _mutex(&mutex) {
        }

      public:
        ~Guard() noexcept {
            if (_mutex == nullptr) {
                return;
            }
            _mutex->_unlock();
        }

        Guard(Guard&& tmp) noexcept 
            : _mutex(tmp._mutex)
        {
            tmp._mutex = nullptr;
        }

        Guard(const Guard&) = delete;
        void operator=(const Guard&) = delete;
    };

    fn lock() noexcept -> LockResult<Guard> {
        using Result = LockResult<Guard>;

        let stat = _lock();
        return stat == LockError::Success ? Result::Ok(Guard(*this)) : Result::Err(stat);
    }

  private:
   fn _init()       -> LockError;
   fn _destroy()    -> void;
   fn _lock()       -> LockError;
   fn _unlock()     -> LockError;
};

using MutexGuard = Mutex::Guard;

}
