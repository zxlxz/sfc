

#include "sfc/io.h"
#include "sfc/serde.h"
#include "sfc/test.h"

namespace sfc::serde::ser {

SFC_TEST(ser_bool) {
  {
    auto node = ser::serialize(true);
    test::assert_true(node.is_bool());
    test::assert_true(node.as_bool());
  }

  {
    auto node = ser::serialize(false);
    test::assert_true(node.is_bool());
    test::assert_false(node.as_bool());
  }
}

SFC_TEST(ser_int) {
  {
    auto node = ser::serialize(-123);
    test::assert_true(node.is_int());
    test::assert_eq(node.as_int(), -123);
  }

  {
    auto node = ser::serialize(12345678910U);
    test::assert_true(node.is_int());
    test::assert_eq(node.as_int(), static_cast<i64>(12345678910U));
  }
}

SFC_TEST(ser_flt) {
  {
    auto node = ser::serialize(-1.23);
    test::assert_true(node.is_flt());
    test::assert_flt_eq(node.as_flt(), -1.23);
  }

  {
    auto node = ser::serialize(+1.23f);
    test::assert_true(node.is_flt());
    test::assert_flt_eq(node.as_flt(), +1.23f);
  }
}

SFC_TEST(ser_str) {
  {
    auto node = ser::serialize(Str{""});
    test::assert_true(node.is_str());
    test::assert_eq(node.as_str(), Str{""});
  }

  {
    auto node = ser::serialize(Str{"abc"});
    test::assert_true(node.is_str());
    test::assert_eq(node.as_str(), Str{"abc"});
  }
}

SFC_TEST(ser_array) {
  {
    const int v[] = {1, 2, 3};
    auto node = ser::serialize(v);
    test::assert_true(node.is_list());

    const auto& list = node.as_list();
    test::assert_eq(list.len(), 3U);

    test::assert_true(list[0].is_int());

    test::assert_eq(list[0].as_int(), 1);
    test::assert_eq(list[1].as_int(), 2);
    test::assert_eq(list[2].as_int(), 3);
  }
}

SFC_TEST(ser_vector) {
  {
    auto v = Vec<int>{};
    v.push(1);
    v.push(2);
    v.push(3);

    auto node = ser::serialize(v);
    test::assert_true(node.is_list());

    const auto& list = node.as_list();
    test::assert_eq(list.len(), 3U);

    test::assert_true(list[0].is_int());

    test::assert_eq(list[0].as_int(), 1);
    test::assert_eq(list[1].as_int(), 2);
    test::assert_eq(list[2].as_int(), 3);
  }
}

SFC_TEST(ser_dict) {
  {
    auto v = VecMap<String, int>{};
    v.insert(String::from("a"), 1);
    v.insert(String::from("b"), 2);

    auto node = ser::serialize(v);
    test::assert_true(node.is_dict());

    auto& dict = node.as_dict();
    test::assert_eq(dict.len(), 2U);
  }
}

namespace {
enum class TestEnum1 { A, B, C };
SFC_ENUM(TestEnum1, A, B, C);
}  // namespace

SFC_TEST(ser_enum) {
  const auto node = serialize(TestEnum1::B);
  test::assert_true(node.is_str());
  test::assert_eq(node.as_str(), "B");
}

namespace {
struct TestStruct {
  int a;
  float b[2];
};
SFC_STRUCT(TestStruct, a, b);
}  // namespace

SFC_TEST(ser_struct) {
  const auto val = TestStruct{
      1,
      {2, 3},
  };
  const auto node = serialize(val);
  test::assert_true(node.is_dict());
}

}  // namespace sfc::serde::ser
