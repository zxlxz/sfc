#include "config.h"
#include "ustd/sync.h"

namespace ustd::sync
{

fn Mutex::_init() -> LockError {
    static_assert(sizeof(mtx_t)  == sizeof(::mtx_t),  "ustd::sync::Mutex: mtx size not equal");
    static_assert(alignof(mtx_t) == alignof(::mtx_t), "ustd::sync::Mutex: mtx align not equal");

    _stat = mtx_init((::mtx_t*)(&_mtx), mtx_palin);
    return stat == 0 ? LockError::Success : Other;
}

fn Mutex::_destroy() -> void {
    if (_stat != 0) {
        return;
    }
    mtx_destroy((::mtx_t*)(&_mtx));
}

fn Mutex::_lock() -> LockError {
    let stat = mtx_lock((::mtx_t*)(&_mtx));
    return stat == 0 ? LockError::Success : Other;
}

fn Mutex::_unlock() -> LockError {
    let stat = mtx_unlock((::mtx_t*)(&_mtx));
    return stat == 0 ? LockError::Success : Other;
}

}

