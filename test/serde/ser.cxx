

#include "sfc/io.h"
#include "sfc/serde.h"
#include "sfc/test.h"

namespace sfc::serde {

SFC_TEST(ser_bool) {
  {
    auto node = serde::serialize(true);
    panicking::assert_true(node.is_bool());
    panicking::assert_true(node.as_bool());
  }

  {
    auto node = serde::serialize(false);
    panicking::assert_true(node.is_bool());
    panicking::assert_false(node.as_bool());
  }
}

SFC_TEST(ser_int) {
  {
    auto node = serde::serialize(-123);
    panicking::assert_true(node.is_int());
    panicking::assert_eq(node.as_int(), -123);
  }

  {
    auto node = serde::serialize(12345678910U);
    panicking::assert_true(node.is_int());
    panicking::assert_eq(node.as_int(), static_cast<i64>(12345678910U));
  }
}

SFC_TEST(ser_flt) {
  {
    auto node = serde::serialize(-1.23);
    panicking::assert_true(node.is_flt());
    panicking::assert_flt_eq(node.as_flt(), -1.23);
  }

  {
    auto node = serde::serialize(+1.23f);
    panicking::assert_true(node.is_flt());
    panicking::assert_flt_eq(node.as_flt(), +1.23f);
  }
}

SFC_TEST(ser_str) {
  {
    auto node = serde::serialize(Str{""});
    panicking::assert_true(node.is_str());
    panicking::assert_eq(node.as_str(), Str{""});
  }

  {
    auto node = serde::serialize(Str{"abc"});
    panicking::assert_true(node.is_str());
    panicking::assert_eq(node.as_str(), Str{"abc"});
  }
}

SFC_TEST(ser_array) {
  {
    const int v[] = {1, 2, 3};
    auto node = serde::serialize(v);
    panicking::assert_true(node.is_list());

    const auto& list = node.as_list();
    panicking::assert_eq(list.len(), 3U);

    panicking::assert_true(list[0].is_int());

    panicking::assert_eq(list[0].as_int(), 1);
    panicking::assert_eq(list[1].as_int(), 2);
    panicking::assert_eq(list[2].as_int(), 3);
  }
}

SFC_TEST(ser_vector) {
  {
    auto v = Vec<int>{};
    v.push(1);
    v.push(2);
    v.push(3);

    auto node = serde::serialize(v);
    panicking::assert_true(node.is_list());

    const auto& list = node.as_list();
    panicking::assert_eq(list.len(), 3U);

    panicking::assert_true(list[0].is_int());

    panicking::assert_eq(list[0].as_int(), 1);
    panicking::assert_eq(list[1].as_int(), 2);
    panicking::assert_eq(list[2].as_int(), 3);
  }
}

SFC_TEST(ser_dict) {
  {
    auto v = VecMap<String, int>{};
    v.insert(String::from("a"), 1);
    v.insert(String::from("b"), 2);

    auto node = serde::serialize(v);
    panicking::assert_true(node.is_dict());

    auto& dict = node.as_dict();
    panicking::assert_eq(dict.len(), 2U);
  }
}

namespace {
enum class TestEnum1 { A, B, C };
SFC_ENUM(TestEnum1, A, B, C);
}  // namespace

SFC_TEST(ser_enum) {
  const auto node = serialize(TestEnum1::B);
  panicking::assert_true(node.is_str());
  panicking::assert_eq(node.as_str(), "B");
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
  panicking::assert_true(node.is_dict());
}

}  // namespace sfc::serde::ser
