#include "sfc/core/future.h"


namespace sfc::future {

auto Frame::done() const noexcept -> bool {
  return __builtin_coro_done(_addr);
}

void Frame::resume() const noexcept {
  __builtin_coro_resume(_addr);
}

void Frame::destroy() const noexcept {
  __builtin_coro_destroy(_addr);
}

template <usize ALIGN>
auto Frame::promise() const noexcept-> void* {
  return __builtin_coro_promise(_addr, ALIGN, false);
}

template <usize ALIGN>
auto Frame::from_promise(void* promise) noexcept-> Frame {
  return Frame{__builtin_coro_promise(promise, ALIGN, true)};
}

template auto Frame::promise<1>() const noexcept -> void*;
template auto Frame::promise<2>() const noexcept -> void*;
template auto Frame::promise<4>() const noexcept -> void*;
template auto Frame::promise<8>() const noexcept -> void*;
template auto Frame::promise<16>() const noexcept -> void*;

template auto Frame::from_promise<1>(void* promise) noexcept -> Frame;
template auto Frame::from_promise<2>(void* promise) noexcept -> Frame;
template auto Frame::from_promise<4>(void* promise) noexcept -> Frame;
template auto Frame::from_promise<8>(void* promise) noexcept -> Frame;
template auto Frame::from_promise<16>(void* promise) noexcept -> Frame;

}  // namespace sfc::future
