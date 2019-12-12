#include "rc.inl"

#include "rc/thread.h"

namespace rc::thread {

pub auto _spawn(FnBox<void()> callback) -> Thread;
pub auto _join() -> void;

pub auto Thread::yield_now() -> void {
}

pub auto Thread::sleep(time::Duration dur) -> void{
}

}