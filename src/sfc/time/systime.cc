#include "sfc/time/systime.h"

#include "sfc/sys/time.h"

namespace sfc::time {

namespace sys_imp = sys::time;

auto SysTime::now() -> SysTime {
  const auto nanos = sys_imp::system_now();
  return SysTime{nanos};
}

}  // namespace sfc::time
