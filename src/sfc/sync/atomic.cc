#if defined(_MSC_VER) && !defined(__clang__)

#include "sfc/sync/atomic.h"

#include <intrin.h>

namespace sfc::sync {

template <class T>
auto __atomic_load_n(T* ptr, int) -> T {
  if constexpr (sizeof(T) == 1) {
    return (T)_InterlockedCompareExchange8((char volatile*)(ptr), 0, 0);
  } else if constexpr (sizeof(T) == 2) {
    return (T)_InterlockedCompareExchange16((short volatile*)(ptr), 0, 0);
  } else if constexpr (sizeof(T) == 4) {
    return (T)_InterlockedCompareExchange((long volatile*)(ptr), 0, 0);
  } else if constexpr (sizeof(T) == 8) {
    return (T)_InterlockedCompareExchange64((__int64 volatile*)(ptr), 0, 0);
  }
}

template <class T>
void __atomic_store_n(T* ptr, T val, int) {
  if constexpr (sizeof(T) == 1) {
    _InterlockedExchange8((char volatile*)(ptr), (char)val);
  } else if constexpr (sizeof(T) == 2) {
    _InterlockedExchange16((short volatile*)(ptr), (short)val);
  } else if constexpr (sizeof(T) == 4) {
    _InterlockedExchange((long volatile*)(ptr), (long)val);
  } else if constexpr (sizeof(T) == 8) {
    _InterlockedExchange64((__int64 volatile*)(ptr), (__int64)val);
  }
}

template <class T>
auto __atomic_exchange_n(T* ptr, T val, int) -> T {
  if constexpr (sizeof(T) == 1) {
    return (T)_InterlockedExchange8((char volatile*)(ptr), (char)val);
  } else if constexpr (sizeof(T) == 2) {
    return (T)_InterlockedExchange16((short volatile*)(ptr), (short)val);
  } else if constexpr (sizeof(T) == 4) {
    return (T)_InterlockedExchange((long volatile*)(ptr), (long)val);
  } else if constexpr (sizeof(T) == 8) {
    return (T)_InterlockedExchange64((__int64 volatile*)(ptr), (__int64)val);
  }
}

template <class T>
auto __atomic_fetch_add(T* ptr, T val, int) -> T {
  if constexpr (sizeof(T) == 1) {
    return (T)_InterlockedExchangeAdd8((char volatile*)(ptr), (char)val);
  } else if constexpr (sizeof(T) == 2) {
    return (T)_InterlockedExchangeAdd16((short volatile*)(ptr), (short)val);
  } else if constexpr (sizeof(T) == 4) {
    return (T)_InterlockedExchangeAdd((long volatile*)(ptr), (long)val);
  } else if constexpr (sizeof(T) == 8) {
    return (T)_InterlockedExchangeAdd64((__int64 volatile*)(ptr), (__int64)val);
  }
}

template <class T>
auto __atomic_fetch_sub(T* ptr, T val, int) -> T {
  if constexpr (sizeof(T) == 1) {
    return (T)_InterlockedExchangeAdd8((char volatile*)(ptr), -(char)val);
  } else if constexpr (sizeof(T) == 2) {
    return (T)_InterlockedExchangeAdd16((short volatile*)(ptr), -(short)val);
  } else if constexpr (sizeof(T) == 4) {
    return (T)_InterlockedExchangeAdd((long volatile*)(ptr), -(long)val);
  } else if constexpr (sizeof(T) == 8) {
    return (T)_InterlockedExchangeAdd64((__int64 volatile*)(ptr), -(__int64)val);
  }
}

#define IMPL_ATOMIC(T)                                \
  template auto __atomic_load_n(T*, int) -> T;        \
  template auto __atomic_store_n(T*, T, int) -> void; \
  template auto __atomic_exchange_n(T*, T, int) -> T; \
  template auto __atomic_fetch_add(T*, T, int) -> T;  \
  template auto __atomic_fetch_sub(T*, T, int) -> T;

IMPL_ATOMIC(char);
IMPL_ATOMIC(short);
IMPL_ATOMIC(int);
IMPL_ATOMIC(long);
IMPL_ATOMIC(long long);

IMPL_ATOMIC(unsigned char);
IMPL_ATOMIC(unsigned short);
IMPL_ATOMIC(unsigned int);
IMPL_ATOMIC(unsigned long);
IMPL_ATOMIC(unsigned long long);

IMPL_ATOMIC(bool);
IMPL_ATOMIC(void*);

#undef IMPL_ATOMIC

}  // namespace sfc::sync

#endif
