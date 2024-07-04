#include "sfc/math.h"
#include "sfc/test.h"

namespace sfc::math {

#if 0
SFC_TEST(sin_tbl) {
  for (auto i = 0; i < 20; ++i) {
    f32 angles[] = {static_cast<f32>(i * 30), -static_cast<f32>(i * 30)};
    for (auto deg_ang : angles) {
      auto rad_ang = math::deg2rad(deg_ang);
      auto sin_val = fast_sin(rad_ang);
      auto cos_val = fast_cos(rad_ang);

      auto asin_val = math::rad2deg(fast_asin(sin_val));
      auto acos_val = math::rad2deg(fast_acos(cos_val));
    }
  }
}
#endif

}  // namespace sfc::math
