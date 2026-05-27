

#include "sfc/test/test.h"
#include "sfc/collections.h"
#include "sfc/serde/json.h"

namespace sfc::serde::json::test {

SFC_TEST(serialize_simple) {
  // bool
  sfc::assert_eq(json::to_string(true), "true");
  sfc::assert_eq(json::to_string(false), "false");

  // int
  sfc::assert_eq(json::to_string(0), "0");
  sfc::assert_eq(json::to_string(123), "123");
  sfc::assert_eq(json::to_string(-123), "-123");

  // float
  sfc::assert_eq(json::to_string(0.0), "0.000000");
  sfc::assert_eq(json::to_string(1.23), "1.230000");
  sfc::assert_eq(json::to_string(-1.23), "-1.230000");

  // str
  sfc::assert_eq(json::to_string(Str{""}), "\"\"");
  sfc::assert_eq(json::to_string(Str{"abc"}), "\"abc\"");
}

SFC_TEST(serialize_seq) {
  const int seq[] = {1, 2, 3};
  sfc::assert_eq(json::to_string(seq), "[1,2,3]");
}

SFC_TEST(serialize_map) {
  auto map = Dict<Str, int>{};
  map.insert(Str{"a"}, 1);
  map.insert(Str{"b"}, 2);
  sfc::assert_eq(json::to_string(map), R"({"a":1,"b":2})");
}

SFC_TEST(deserialize_simple) {
  // null
  { sfc::assert_eq(Deserializer{"null"}.deserialize_null().is_ok(), true); }

  // bool
  {
    sfc::assert_eq(Deserializer{"true"}.deserialize_bool().ok(), Option{true});
    sfc::assert_eq(Deserializer{"false"}.deserialize_bool().ok(), Option{false});
  }

  // int
  {
    sfc::assert_eq(Deserializer{"0"}.deserialize_i64().ok(), Option{0});
    sfc::assert_eq(Deserializer{"123"}.deserialize_i64().ok(), Option{123});
    sfc::assert_eq(Deserializer{"-123"}.deserialize_i64().ok(), Option{-123});
  }

  // float
  {
    sfc::assert_eq(Deserializer{"0.0"}.deserialize_f64().ok(), Option{0.0});
    sfc::assert_eq(Deserializer{"1.23"}.deserialize_f64().ok(), Option{1.23});
    sfc::assert_eq(Deserializer{"-1.23"}.deserialize_f64().ok(), Option{-1.23});
  }

  // str
  {
    sfc::assert_eq(Deserializer{R"("")"}.deserialize_str().ok(), Option{""});
    sfc::assert_eq(Deserializer{R"("abc")"}.deserialize_str().ok(), Option{"abc"});
  }
}

SFC_TEST(deserialize_seq) {
  const Str s = "[0,1,2]";
  const int vals[] = {0, 1, 2};
  auto visit = [&](auto& seq) -> Result<> {
    for (auto i = 0U; i < 3; ++i) {
      sfc::assert_eq(seq.has_next(), true);
      const auto val = seq.template next_element<int>().ok();
      sfc::assert_eq(val, Option{vals[i]});
    }
    sfc::assert_eq(seq.has_next(), false);
    return {};
  };

  auto des = Deserializer{s};
  auto ret = des.deserialize_seq(visit);
  sfc::assert_eq(ret.is_ok(), true);
}

SFC_TEST(deserialize_map) {
  const auto s = Str{R"({"a":1,"b":2})"};
  const Str keys[] = {"a", "b"};
  const int vals[] = {1, 2};

  auto visit = [&](auto& map) -> Result<> {
    for (auto i = 0U; i < 2; ++i) {
      const auto key = map.next_key();
      sfc::assert_eq(auto{key}.ok(), Option{keys[i]});

      const auto val = map.template next_value<int>();
      sfc::assert_eq(auto{val}.ok(), Option{vals[i]});
    }
    sfc::assert_eq(map.has_next(), false);
    return {};
  };

  auto des = Deserializer{s};
  sfc::assert_eq(des.deserialize_map(visit).is_ok(), true);
}

}  // namespace sfc::serde::json::test
