#include "sfc/io.h"
#include "sfc/serde.h"
#include "sfc/test.h"

namespace sfc::serde::test {

#if 0
SFC_TEST(ser_bool) {
  {
    auto node = serde::serialize(true);
    panicking::expect(node.is<bool>());
    panicking::expect(node.as<bool>());
  }

  {
    auto node = serde::serialize(false);
    panicking::expect(node.is<bool>());
    panicking::expect(!node.as<bool>());
  }
}

SFC_TEST(ser_int) {
  {
    auto node = serde::serialize(-123);
    panicking::expect(node.is<i64>());
    panicking::expect_eq(node.as<i64>(), -123);
  }

  {
    auto node = serde::serialize(12345678910U);
    panicking::expect(node.is<i64>());
    panicking::expect_eq(node.as<i64>(), static_cast<i64>(12345678910U));
  }
}

SFC_TEST(ser_flt) {
  {
    auto node = serde::serialize(-1.23);
    panicking::expect(node.is<f64>());
    panicking::expect_flt_eq(node.as<f64>(), -1.23);
  }

  {
    auto node = serde::serialize(+1.23);
    panicking::expect(node.is<f64>());
    panicking::expect_flt_eq(node.as<f64>(), +1.23);
  }
}

SFC_TEST(ser_str) {
  {
    auto node = serde::serialize(Str{""});
    panicking::expect(node.is<String>());
    panicking::expect_eq(node.as<String>(), Str{""});
  }

  {
    auto node = serde::serialize(Str{"abc"});
    panicking::expect(node.is<String>());
    panicking::expect_eq(node.as<String>(), Str{"abc"});
  }
}

SFC_TEST(ser_array) {
  {
    const int v[] = {1, 2, 3};
    auto node = serde::serialize(Slice{v});
    panicking::expect(node.is<List>());

    const auto& list = node.as<List>();
    panicking::expect_eq(list.len(), 3U);

    panicking::expect(list[0].is<i64>());

    panicking::expect_eq(list[0].as<i64>(), 1);
    panicking::expect_eq(list[1].as<i64>(), 2);
    panicking::expect_eq(list[2].as<i64>(), 3);
  }
}

SFC_TEST(ser_vector) {
  {
    auto v = Vec<int>{};
    v.push(1);
    v.push(2);
    v.push(3);

    auto node = serde::serialize(v);
    panicking::expect(node.is<List>());

    const auto& list = node.as<List>();
    panicking::expect_eq(list.len(), 3U);

    panicking::expect(list[0].is<i64>());

    panicking::expect_eq(list[0].as<i64>(), 1);
    panicking::expect_eq(list[1].as<i64>(), 2);
    panicking::expect_eq(list[2].as<i64>(), 3);
  }
}

SFC_TEST(ser_dict) {
  {
    auto v = collections::VecMap<String, int>{};
    v.insert(String::from("a"), 1);
    v.insert(String::from("b"), 2);

    auto node = serde::serialize(v);
    panicking::expect(node.is<Dict>());

    auto& dict = node.as<Dict>();
    panicking::expect_eq(dict.len(), 2U);
  }
}

struct Ser1 {
  int a;
  float b[2];

  void fmt(auto& f) const {
    auto x = f.debug_struct();
    x.field("a", a);
    x.field("b", b);
  }

  auto serialize(auto& s) const {
    auto x = s.new_dict();
    x.insert("a", s.ser(a));
    x.insert("b", s.ser(b));
    return x;
  }
};

SFC_TEST(ser_struct) {
  const auto val = Ser1{
      1,
      {2, 3},
  };
  const auto node = serialize(val);
  panicking::expect(node.is<Dict>());
}
#endif

}  // namespace sfc::serde::test
