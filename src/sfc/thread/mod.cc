#include "mod.h"

namespace sfc::thread {

auto Builder::spawn(Box<void()> f) -> JoinHandle {
  const auto thr = sys::Thread::create(mem::move(f));

  return JoinHandle{Thread{thr._id}};
}

JoinHandle::JoinHandle(Thread thr) noexcept : _thr{thr} {}

JoinHandle::JoinHandle() = default;

JoinHandle::JoinHandle(JoinHandle&& other) noexcept : _thr{other._thr} {
  other._thr._id = Thread::INVALID_ID;
}

JoinHandle::~JoinHandle() {
  this->join();
}

auto JoinHandle::operator=(JoinHandle&& other) noexcept -> JoinHandle& {
  auto tmp = static_cast<JoinHandle&&>(*this);
  mem::swap(_thr, other._thr);
  return *this;
}

void JoinHandle::join() {
  if (_thr._id == Thread::INVALID_ID) {
    return;
  }

  sys::Thread{_thr._id}.join();
}

void sleep(time::Duration dur) {
  sys::sleep(dur);
}

void sleep_ms(u32 ms) {
  auto dur = time::Duration::from_millis(ms);
  sys::sleep(dur);
}

}  // namespace sfc::thread
