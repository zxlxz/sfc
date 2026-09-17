

#include "sfc/test/test.h"
#include "sfc/collections.h"
#include "sfc/serde/json.h"

namespace sfc::serde::json::test {

SFC_TEST(serde_bool) {
  // true
  {
    sfc::assert_eq(json::to_string(true), "true");

    auto des = Deserializer::from_str("true");
    sfc::assert_eq(des.deserialize_bool().ok(), Option{true});
  }

  // false
  {
    sfc::assert_eq(json::to_string(false), "false");

    auto des = Deserializer::from_str("false");
    sfc::assert_eq(des.deserialize_bool().ok(), Option{false});
  }
}

SFC_TEST(serde_int) {
  // 123
  {
    sfc::assert_eq(json::to_string(123), "123");

    auto des = Deserializer::from_str("123");
    sfc::assert_eq(des.deserialize_any<i64>().ok(), Option{123});
  }

  // -123 -> i64
  {
    sfc::assert_eq(json::to_string(-123), "-123");

    auto des = Deserializer::from_str("-123");
    sfc::assert_eq(des.deserialize_any<i64>().ok(), Option{-123});
  }

  // -123 -> u64
  {
    sfc::assert_eq(json::to_string(-123), "-123");

    auto des = Deserializer::from_str("-123");
    sfc::assert_eq(des.deserialize_any<u64>().ok(), Option<u64>{});
  }
}

SFC_TEST(serde_flt) {
  // 1.2
  {
    sfc::assert_eq(json::to_string(1.2), "1.200000");

    auto des = Deserializer::from_str("1.2");
    sfc::assert_eq(des.deserialize_any<f64>().ok(), Option{1.2});
  }

  // -1.2
  {
    sfc::assert_eq(json::to_string(-1.2), "-1.200000");

    auto des = Deserializer::from_str("-1.2");
    sfc::assert_eq(des.deserialize_any<f64>().ok(), Option{-1.2});
  }
}

SFC_TEST(serde_str) {
  // abc
  {
    sfc::assert_eq(json::to_string("abc"), "\"abc\"");

    auto des = Deserializer::from_str("\"abc\"");
    sfc::assert_eq(des.deserialize_str().ok(), Option{Str{"abc"}});
  }

  // a b c
  {
    sfc::assert_eq(json::to_string("a b c"), "\"a b c\"");

    auto des = Deserializer::from_str("\"a b c\"");
    sfc::assert_eq(des.deserialize_str().ok(), Option{Str{"a b c"}});
  }
}

SFC_TEST(serde_seq) {
  // [0, 1, 2]
  {
    const int vals[] = {0, 1, 2};
    sfc::assert_eq(json::to_string(vals), "[0, 1, 2]");

    auto des = Deserializer::from_str("[0,1,2]");
    auto list = List<int>::deserialize(des).unwrap();

    sfc::assert_eq(list.len(), 3U);
    for (auto i = 0U; i < 3; ++i) {
      sfc::assert_eq(list[i], vals[i]);
    }
  }
}

SFC_TEST(serde_map) {
  // {"a":1,"b":2}
  {
    auto dict = collections::HashMap<String, int>{};
    dict.insert(String::from("a"), 1);
    dict.insert(String::from("b"), 2);
    sfc::assert_eq(json::to_string(dict), "{\"a\": 1, \"b\": 2}");

    auto des = Deserializer::from_str("{\"a\":1,\"b\":2}");
    dict.clear();
    sfc::assert_eq(dict.len(), 0U);

    dict = collections::HashMap<String, int>::deserialize(des).unwrap();
    sfc::assert_eq(dict.len(), 2U);

    sfc::assert_eq(dict.get(Str{"a"}), Option{1});
    sfc::assert_eq(dict.get(Str{"b"}), Option{2});
  }
}

}  // namespace sfc::serde::json::test
