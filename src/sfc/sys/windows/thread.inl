#pragma once

#include <Windows.h>

namespace sfc::sys::thread {

static auto wchar_to_u8(const wchar_t src[], char dst[], int dst_len) {
  return ::WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, dst_len, nullptr, nullptr);
}

static auto u8_to_wchar(const char src[], wchar_t dst[], int dst_len) {
  return ::MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, dst_len);
}

struct Thread {
  using tid_t = void*;
  static constexpr unsigned kMaxNameLen = 256U;

  tid_t _raw = nullptr;

 public:
  static auto current() -> Thread {
    const auto thrd = ::GetCurrentThread();
    return Thread{thrd};
  }

  static auto start(SIZE_T stack_size, DWORD (*func)(void*), void* data) -> Thread {
    const auto thrd = ::CreateThread(nullptr, stack_size, func, data, 0, nullptr);
    return Thread{thrd};
  }

  explicit operator bool() const {
    return _raw != nullptr;
  }

  auto id() const -> DWORD {
    if (_raw == nullptr) {
      return 0;
    }
    return ::GetThreadId(_raw);
  }

  auto join() -> bool {
    if (_raw == nullptr) {
      return true;
    }

    const auto ret = ::WaitForSingleObject(_raw, INFINITE);
    if (ret != WAIT_OBJECT_0) {
      return false;
    }

    return true;
  }

  auto detach() -> bool {
    if (_raw == nullptr) {
      return true;
    }

    if (!::CloseHandle(_raw)) {
      return false;
    }

    _raw = nullptr;

    return true;
  }

  auto get_name(char* buf, SIZE_T buf_len) const -> SIZE_T {
    if (_raw == nullptr) {
      return 0;
    }

    wchar_t*   wbuf = nullptr;
    const auto hres = ::GetThreadDescription(_raw, &wbuf);
    if (FAILED(hres)) {
      return 0;
    }

    const auto nbytes = wchar_to_u8(wbuf, buf, kMaxNameLen);
    ::LocalFree(wbuf);

    if (nbytes <= 0 || nbytes >= kMaxNameLen) {
      return 0;
    }

    return nbytes;
  }

  auto set_name(const char* name) -> bool {
    if (_raw == nullptr) {
      return false;
    }

    wchar_t    wbuff[kMaxNameLen];
    const auto cnt = u8_to_wchar(name, wbuff, kMaxNameLen);
    if (cnt <= 0 && cnt >= kMaxNameLen) {
      return false;
    }

    const auto hres = ::SetThreadDescription(_raw, wbuff);
    return SUCCEEDED(hres);
  }
};

struct Mutex {
  ::CRITICAL_SECTION _cs{};

 public:
  Mutex(bool recursive = false) {
    (void)recursive;
    ::InitializeCriticalSection(&_cs);
  }

  ~Mutex() noexcept {
    ::DeleteCriticalSection(&_cs);
  }

  void lock() {
    ::EnterCriticalSection(&_cs);
  }

  void unlock() {
    ::LeaveCriticalSection(&_cs);
  }
};

struct Condvar {
  ::CONDITION_VARIABLE _cv{};

 public:
  Condvar() {
    ::InitializeConditionVariable(&_cv);
  }

  ~Condvar() noexcept {
    // No destructor for CONDITION_VARIABLE
  }

  void notify_one() {
    ::WakeConditionVariable(&_cv);
  }

  void notify_all() {
    ::WakeAllConditionVariable(&_cv);
  }

  auto wait(Mutex& mtx) -> bool {
    return ::SleepConditionVariableCS(&_cv, &mtx._cs, INFINITE);
  }

  auto wait_timeout_ms(Mutex& mtx, DWORD millis) -> bool {
    return ::SleepConditionVariableCS(&_cv, &mtx._cs, millis);
  }
};

static inline void sleep_ms(DWORD millis) {
  ::Sleep(millis);
}

}  // namespace sfc::sys::thread
