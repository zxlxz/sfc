#include "sfc/log/global.h"

namespace sfc::log {

auto global() -> Logger<GlobalBackend>& {
  static auto backend = GlobalBackend{};
  static auto logger = Logger<GlobalBackend>{backend};
  return logger;
}

}
