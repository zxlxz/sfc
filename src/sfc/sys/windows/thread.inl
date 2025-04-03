#pragma once

#include <Windows.h>
#undef max
#undef min

#include "sfc/alloc.h"

namespace sfc::sys::thread {

struct Thread {
  HANDLE _thrd;

  static auto start_routine(void* raw) -> DWORD {
    const auto ptr = static_cast<Box<void()>*>(raw);
    auto fun       = Box<Box<void()>>::from_raw(ptr);
    (*fun)();

    return 0U;
  }

 public:
  static auto xnew(usize stack_size, Box<void()> fun) -> Option<Thread> {
    auto bfun = boxed::box(mem::move(fun));

    const auto thrd = ::CreateThread(nullptr, stack_size, &start_routine, bfun.ptr(), 0, nullptr);
    if (thrd == nullptr) {
      return {};
    }

    mem::move(bfun).into_raw();

    return Thread{thrd};
  }

  static auto current() -> Thread {
    const auto thrd = ::GetCurrentThread();
    return Thread{thrd};
  }

  auto raw() const -> HANDLE {
    return _thrd;
  }

  auto name() const -> String {
    wchar_t* wbuff  = nullptr;
    const auto hres = ::GetThreadDescription(_thrd, &wbuff);
    if (FAILED(hres)) {
      return {};
    }

    static constexpr auto kMaxNameLen = 256;
    char buff[kMaxNameLen];
    const auto cnt =
        ::WideCharToMultiByte(CP_UTF8, 0, wbuff, -1, buff, kMaxNameLen, nullptr, nullptr);
    if (cnt <= 0 || cnt >= kMaxNameLen) {
      return {};
    }

    auto res = String{};
    res.push_str(Str{buff, DWORD(cnt)});
    return res;
  }

  auto set_name(cstr_t name) -> bool {
    static constexpr auto kMaxNameLen = 256U;
    wchar_t wbuff[kMaxNameLen];
    const auto cnt = ::MultiByteToWideChar(CP_UTF8, 0, name, -1, wbuff, kMaxNameLen);
    if (cnt <= 0 && cnt >= kMaxNameLen) {
      return false;
    }

    const auto hres = ::SetThreadDescription(_thrd, wbuff);
    return SUCCEEDED(hres);
  }

  auto join() -> bool {
    const auto ret = ::WaitForSingleObject(_thrd, -1);
    return ret != 0;
  }

  auto detach() -> bool {
    const auto ret = ::CloseHandle(_thrd);
    return ret != 0;
  }
};

static inline auto sleep_ms(DWORD millis) -> bool {
  ::Sleep(millis);
  return true;
}

}  // namespace sfc::sys::thread
