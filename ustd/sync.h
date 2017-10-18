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
        Mutex*  _mutex;

      public:
        Guard(Mutex& mutex) noexcept
            : _mutex(&mutex) {
            _mutex->_lock();
        }

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

    fn lock() noexcept -> Guard {
        return {*this};
    }

  private:
   fn _init() -> void;
   fn _destroy() -> void;
   fn _lock() -> void;
   fn _unlock() ->void;
};

}
