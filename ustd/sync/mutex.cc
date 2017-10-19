#include "config.h"

namespace ustd::sync
{

fn _mtx_init(mtx_t *mtx) -> LockError {
    let stat = mtx_init(mtx, mtx_palin);
    return stat == 0 ? LockError::Success : Other;
}

fn _mtx_destroy(mtx_t* mtx) -> void {
    mtx_destroy(mtx);
}

fn _mtx_lock(mtx_t* mtx) -> LockError {
    let stat = mtx_lock(mtx);
    return stat == 0 ? LockError::Success : Other;
}

fn _mtx_unlock(mtx_t* mtx) -> LockError {
    let stat = mtx_unlock(mtx);
    return stat == 0 ? LockError::Success : Other;
}

}

