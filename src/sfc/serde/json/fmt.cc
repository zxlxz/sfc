#include "fmt.h"

namespace sfc::serde {

auto Node::to_json() const -> String {
  return json::format(*this);
}

}  // namespace sfc::serde
