#pragma once

#include <ustd/core.h>
#include <ustd/os/threads.h>

namespace ustd::sync
{

enum LockError
{
    Success,
    WouldBlock,
    Other
};

auto _mtx_init   (os::mtx_t *mtx) -> LockError;
auto _mtx_destroy(os::mtx_t* mtx) -> void;
auto _mtx_lock   (os::mtx_t* mtx) -> LockError;
auto _mtx_unlock (os::mtx_t* mtx) -> LockError;

template<class U>   using LockResult = result::Result<U, typename U::Error>;
template<class T>   class Mutex;
template<class T>   class MutexGuard;

template<class T=int>
class Mutex
{
  public:
    template<class U>
    Mutex(U&& u) noexcept: _handle{}, _value{ as_fwd<U>(u) } {
        (void)_mtx_init(&_handle);
    }

    ~Mutex() noexcept {
        _mtx_destroy(&_handle);
    }

    Mutex(const Mutex&) = delete;
    void operator=(const Mutex&) = delete;

    fn lock() noexcept->LockResult<MutexGuard<T>>;

  protected:
    friend class MutexGuard<T>;

    os::mtx_t   _handle;
    T           _value;
};

template<class T=int>
class MutexGuard
{
public:
    friend class Mutex<T>;
    using Error = LockError;

public:
    ~MutexGuard() noexcept {
        if (_ptr == nullptr) { return; }
        _mtx_unlock(&_ptr->_handle);
    }

    MutexGuard(MutexGuard&& other) noexcept: _ptr(other._ptr) {
        other._ptr = nullptr;
    }

    MutexGuard(const MutexGuard&)       = delete;
    void operator=(const MutexGuard&)   = delete;

    fn deref() const -> T& {
        return _ptr->_value;
    }

    fn operator *() const -> T& {
        return _ptr->_value;
    }

protected:
    Mutex<T>*  _ptr;

    MutexGuard(Mutex<T>& mutex) noexcept : _ptr(&mutex) {
    }

};

template<class T>
fn Mutex<T>::lock() noexcept -> LockResult<MutexGuard<T>> {
    using Result = LockResult<MutexGuard<T>>;

    let stat = _mtx_lock(&_handle);
    return stat == LockError::Success ? Result::Ok(MutexGuard<T>(*this)) : Result::Err(stat);
}

}
