#include "config.h"
#include "ustd/sync.h"

namespace ustd::sync
{

fn Mutex::_init() -> void {
    static_assert(sizeof(mtx_t)  == sizeof(::mtx_t),  "ustd::sync::Mutex: mtx size not equal");
    static_assert(alignof(mtx_t) == alignof(::mtx_t), "ustd::sync::Mutex: mtx align not equal");

    _stat = mtx_init((::mtx_t*)(&_mtx), mtx_palin);
    if (_stat != 0) {
        ustd::panic("nms.thread.Mutex: init failed");
    }
}

fn Mutex::_destroy() -> void {
    if (_stat != 0) {
        return;
    }
    mtx_destroy((::mtx_t*)(&_mtx));
}

fn Mutex::_lock() -> void {
    let stat = mtx_lock((::mtx_t*)(&_mtx));
    if (stat != 0) {
        ustd::panic("nms.sync.MutexGuard: lock failed");
    }
}

fn Mutex::_unlock() -> void {
    let stat = mtx_unlock((::mtx_t*)(&_mtx));
    if (stat != 0) {
        ustd::panic("nms.sync.MutexGuard: unlock failed");
    }
}

}

