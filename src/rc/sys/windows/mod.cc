#include "rc/sys/windows.inl"

namespace rc::sys::windows {

Handle::Handle(void* raw) noexcept: _raw{raw}{}

Handle::Handle(Handle&& other) noexcept : _raw{other._raw} {
  other._raw = nullptr;
}

Handle::~Handle() {
  if (_raw == nullptr) return;

  ::CloseHandle(_raw);
  _raw = nullptr;
}

}  // namespace rc::sys
