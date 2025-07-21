#include "mutex.h"

#include "sfc/sys/sync.h"

namespace sfc::sync {

namespace sys_imp = sys::sync;

struct Mutex::Inn {
  sys_imp::mutex_t _raw;

 public:
#ifdef _WIN32
  Inn() {
    sys_imp::init(_raw);
  }

  ~Inn() {
    sys_imp::drop(_raw);
  }
#endif
};

Mutex::Mutex() : _inn{Box<Inn>::xnew()} {}

Mutex::~Mutex() {}

Mutex::Mutex(Mutex&&) noexcept = default;

Mutex& Mutex::operator=(Mutex&&) noexcept = default;

auto Mutex::lock() -> Guard {
  panicking::assert(_inn, "Mutex::lock: on a dropped object");
  return Guard{*_inn};
}

Mutex::Guard::Guard(Inn& mtx) : _mtx{&mtx} {
  sys_imp::lock(mtx._raw);
}

Mutex::Guard::~Guard() {
  if (!_mtx) {
    return;
  }
  sys_imp::unlock(_mtx->_raw);
}

}  // namespace sfc::sync
