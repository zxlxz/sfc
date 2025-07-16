#pragma once

#include <Windows.h>

namespace sfc::sys::thread {

using thread_ret_t = DWORD;

static auto wchar_to_u8(const wchar_t src[], char dst[], int dst_len) -> DWORD {
  const auto cnt = ::WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, dst_len, nullptr, nullptr);
  return cnt > 0 ? static_cast<DWORD>(cnt) : 0;
}

static auto u8_to_wchar(const char src[], wchar_t dst[], int dst_len) -> DWORD {
  const auto cnt = ::MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, dst_len);
  return cnt > 0 ? static_cast<DWORD>(cnt) : 0;
}

struct Thread {
  void* _handle{nullptr};

 public:
  static auto current() -> Thread {
    const auto thrd = ::GetCurrentThread();
    return Thread{thrd};
  }

  static auto start(SIZE_T stack_size, DWORD (*func)(void*), void* data) -> Thread {
    const auto thrd = ::CreateThread(nullptr, stack_size, func, data, 0, nullptr);
    return Thread{thrd};
  }

  static auto from_raw(SIZE_T raw) -> Thread {
    return Thread{reinterpret_cast<void*>(raw)};
  }

  auto raw() const -> SIZE_T {
    return reinterpret_cast<SIZE_T>(_handle);
  }

  explicit operator bool() const {
    return _handle != nullptr;
  }

  static auto get_tid() const -> DWORD {
    return ::GetCurrentThreadId();
  }

  auto join() -> bool {
    if (_handle == nullptr) {
      return true;
    }

    const auto ret = ::WaitForSingleObject(_handle, INFINITE);
    if (ret != WAIT_OBJECT_0) {
      return false;
    }

    return true;
  }

  auto detach() -> bool {
    if (_handle == nullptr) {
      return true;
    }

    if (!::CloseHandle(_handle)) {
      return false;
    }

    _handle = nullptr;

    return true;
  }

  auto get_name(char* buf, SIZE_T buf_len) const -> SIZE_T {
    if (_handle == nullptr) {
      return 0;
    }

    wchar_t*   wbuf = nullptr;
    const auto hres = ::GetThreadDescription(_handle, &wbuf);
    if (FAILED(hres)) {
      return 0;
    }

    const auto nbytes = wchar_to_u8(wbuf, buf, buf_len);
    ::LocalFree(wbuf);

    if (nbytes <= 0 || nbytes >= buf_len) {
      return 0;
    }

    return nbytes;
  }

  static auto set_name(const char* name) -> bool {
    static constexpr auto kMaxNameLen = 256U;

    wchar_t    wbuff[kMaxNameLen];
    const auto cnt = u8_to_wchar(name, wbuff, kMaxNameLen);
    if (cnt <= 0 || cnt >= kMaxNameLen) {
      return false;
    }

    const auto thrd = ::GetCurrentThread();
    const auto hres = ::SetThreadDescription(thrd, wbuff);
    return SUCCEEDED(hres);
  }
};

struct Mutex {
  ::CRITICAL_SECTION _cs{};

 public:
  Mutex() {
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

  ~Condvar() noexcept = default;

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
