

#include "sfc/test/test.h"
#include "sfc/collections.h"
#include "sfc/serde/json.h"

namespace sfc::serde::json::test {

SFC_TEST(serialize_simple) {
  // bool
  sfc::expect_eq(json::to_string(true), "true");
  sfc::expect_eq(json::to_string(false), "false");

  // int
  sfc::expect_eq(json::to_string(0), "0");
  sfc::expect_eq(json::to_string(123), "123");
  sfc::expect_eq(json::to_string(-123), "-123");

  // float
  sfc::expect_eq(json::to_string(0.0), "0.000000");
  sfc::expect_eq(json::to_string(1.23), "1.230000");
  sfc::expect_eq(json::to_string(-1.23), "-1.230000");

  // str
  sfc::expect_eq(json::to_string(Str{""}), "\"\"");
  sfc::expect_eq(json::to_string(Str{"abc"}), "\"abc\"");
}

SFC_TEST(serialize_seq) {
  const int seq[] = {1, 2, 3};
  sfc::expect_eq(json::to_string(seq), "[1,2,3]");
}

SFC_TEST(serialize_map) {
  auto map = collections::HashMap<Str, int>{};
  map.insert(Str{"a"}, 1);
  map.insert(Str{"b"}, 2);
  sfc::expect_eq(json::to_string(map), R"({"a":1,"b":2})");
}

SFC_TEST(deserialize_simple) {
  // null
  { sfc::expect_true(Deserializer{"null"}.deserialize_null().is_ok()); }

  // bool
  {
    sfc::expect_eq(Deserializer{"true"}.deserialize_bool().ok(), Option{true});
    sfc::expect_eq(Deserializer{"false"}.deserialize_bool().ok(), Option{false});
  }

  // int
  {
    sfc::expect_eq(Deserializer{"0"}.deserialize_int<int>().ok(), Option{0});
    sfc::expect_eq(Deserializer{"123"}.deserialize_int<int>().ok(), Option{123});
    sfc::expect_eq(Deserializer{"-123"}.deserialize_int<int>().ok(), Option{-123});
  }

  // float
  {
    sfc::expect_eq(Deserializer{"0.0"}.deserialize_flt<double>().ok(), Option{0.0});
    sfc::expect_eq(Deserializer{"1.23"}.deserialize_flt<double>().ok(), Option{1.23});
    sfc::expect_eq(Deserializer{"-1.23"}.deserialize_flt<double>().ok(), Option{-1.23});
  }

  // str
  {
    sfc::expect_eq(Deserializer{R"("")"}.deserialize_str().ok(), Option{""});
    sfc::expect_eq(Deserializer{R"("abc")"}.deserialize_str().ok(), Option{"abc"});
  }
}

SFC_TEST(deserialize_seq) {
  const Str s = "[0,1,2]";
  const int vals[] = {0, 1, 2};
  auto visit = [&](auto& seq) -> Result<> {
    for (auto i = 0U; i < 3; ++i) {
      const auto val = seq.template next_element<int>();
      sfc::expect_true(val.is_ok());
      sfc::expect_eq(*val, vals[i]);
    }
    sfc::expect_false(seq.has_next());
    return {};
  };

  auto des = Deserializer{s};
  sfc::expect_true(des.deserialize_seq(visit).is_ok());
}

SFC_TEST(deserialize_map) {
  const auto s = Str{R"({"a":1,"b":2})"};
  const Str keys[] = {"a", "b"};
  const int vals[] = {1, 2};

  auto visit = [&](auto& map) -> Result<> {
    for (auto i = 0U; i < 2; ++i) {
      const auto key = map.template next_key<Str>();
      sfc::expect_true(key.is_ok());
      sfc::expect_eq(*key, keys[i]);

      const auto val = map.template next_value<int>();
      sfc::expect_true(val.is_ok());
      sfc::expect_eq(*val, vals[i]);
    }
    sfc::expect_false(map.has_next());
    return {};
  };

  auto des = Deserializer{s};
  sfc::expect_true(des.deserialize_map(visit).is_ok());
}

}  // namespace sfc::serde::json::test
