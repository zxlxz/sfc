#include <Windows.h>
#include <process.h>
#undef min
#undef max

#include "sfc/sys/windows/thread.h"

namespace sfc::sys::windows {

Thread::Thread(void* handle) noexcept : _handle{handle} {}

struct ThreadData {
  Box<void()> _fun;
};

static unsigned __stdcall start_routine(void* p) {
  auto data = ptr::cast<ThreadData>(p);
  auto func = mem::move(data->_fun);
  delete data;

  (func)();
  return 0;
}

auto Thread::spawn(usize stack_size, Box<void()> obj) -> Thread {
  auto data = new ThreadData{mem::move(obj)};

  const auto thrd = ::_beginthreadex(nullptr, unsigned(stack_size), &start_routine, data, 0, nullptr);
  if (thrd == 0) {
    delete data;
    return {};
  }

  const auto handle = reinterpret_cast<void*>(thrd);
  return Thread{handle};
}

auto Thread::is_valid() const -> bool {
  return _handle != nullptr && _handle != INVALID_HANDLE_VALUE;
}

void Thread::join() {
  if (!this->is_valid()) return;

  const auto ret = ::WaitForSingleObject(_handle, INFINITE);
  ::CloseHandle(_handle);
  sfc::assert_(ret == WAIT_OBJECT_0, "Thread::join: WaitForSingleObject failed, err={}", ::GetLastError());
}

void Thread::detach() {
  if (!this->is_valid()) return;
  (void)::CloseHandle(_handle);
}

auto ThisThread::id() noexcept -> u32 {
  const auto tid = ::GetCurrentThreadId();
  return u32(tid);
}

auto ThisThread::set_name(const char* name) -> bool {
  if (name == nullptr) {
    return true;
  }

  wchar_t wbuf[256];
  auto i = usize{0};
  for (; i < sizeof(wbuf) / sizeof(wbuf[0]) - 1 && name[i] != '\0'; ++i) {
    wbuf[i] = wchar_t(name[i]);
  }
  wbuf[i] = L'\0';

  const auto thrd = ::GetCurrentThread();
  const auto hres = ::SetThreadDescription(thrd, wbuf);
  return SUCCEEDED(hres);
}

void ThisThread::yield() {
  (void)::SwitchToThread();
}

void ThisThread::sleep(time::Duration dur) noexcept {
  const auto millis = num::saturating_cast<DWORD>(dur.as_millis());

  if (millis == 0) {
    ::SwitchToThread();
    return;
  }

  ::Sleep(millis);
}

}  // namespace sfc::sys::windows
