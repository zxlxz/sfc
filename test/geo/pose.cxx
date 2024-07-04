#include "sfc/geo.h"
#include "sfc/test.h"

namespace sfc::geo {

SFC_TEST(pose_l2g) {
  const auto p1 = Point{0, 0};
  const auto p2 = Point{1, 1};

  {
    const auto r = Pose{{1, 1}, Angle{0}.to_vec()};
    const auto g1 = r.trans_l2g()(p1);
    const auto g2 = r.trans_l2g()(p2);
    test::assert_eq(g1, Point{1, 1});
    test::assert_eq(g2, Point{2, 2});
  }

  {
    const auto r = Pose{{1, 0}, Angle{PI / 2}.to_vec()};
    const auto g1 = r.trans_l2g()(p1);
    const auto g2 = r.trans_l2g()(p2);
    test::assert_true((g1 - Point{1, 0}).length() < geo::EPSILON);
    test::assert_true((g2 - Point{0, 1}).length() < geo::EPSILON);
  }
}

SFC_TEST(pose_g2l) {}

SFC_TEST(pose_trans_to) {
  {
    const auto r1 = Pose{{1, 0}, {0, 1}};
    const auto r2 = Pose{{2, 0}, {0, 1}};
    const auto t12 = r1.trans_to(r2);

    const auto p1 = Point{1, 0};
    const auto q1 = t12(p1);
    test::assert_true((q1 - Point{1, 1}).length() < geo::EPSILON);
  }

  {
    const auto g = Point{1, 1};

    auto r = Pose{{0, 0}, Angle{0}.to_vec()};
    auto p = g;
    for (auto i = 0; i < 10; ++i) {
      const auto f = static_cast<f32>(i);
      const auto s = Pose{{0, 0.1 * f}, Angle{0.01 * f}.to_vec()};
      const auto t = r.trans_to(s);
      p = t(p);

      const auto h = s.trans_l2g()(p);
      test::assert_true((g - h).length() < geo::EPSILON);
      r = s;
    }
  }
}

}  // namespace sfc::geo
