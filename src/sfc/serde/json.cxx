

#include "sfc/serde.h"
#include "sfc/test.h"
#include "sfc/collections.h"

namespace sfc::serde::json::test {

SFC_TEST(serialize_simple) {
  // bool
  panicking::expect_eq(json::to_string(true), "true");
  panicking::expect_eq(json::to_string(false), "false");

  // int
  panicking::expect_eq(json::to_string(0), "0");
  panicking::expect_eq(json::to_string(123), "123");
  panicking::expect_eq(json::to_string(-123), "-123");

  // float
  panicking::expect_eq(json::to_string(0.0), "0.000000");
  panicking::expect_eq(json::to_string(1.23), "1.230000");
  panicking::expect_eq(json::to_string(-1.23), "-1.230000");

  // str
  panicking::expect_eq(json::to_string(Str{""}), "\"\"");
  panicking::expect_eq(json::to_string(Str{"abc"}), "\"abc\"");
}

SFC_TEST(serialize_seq) {
  const int seq[] = {1, 2, 3};
  panicking::expect_eq(json::to_string(seq), "[1,2,3]");
}

SFC_TEST(serialize_map) {
  auto map = collections::HashMap<Str, int>{};
  map.insert(Str{"a"}, 1);
  map.insert(Str{"b"}, 2);
  panicking::expect_eq(json::to_string(map), "{\"a\":1,\"b\":2}");
}

SFC_TEST(deserialize_simple) {
  // null
  { panicking::expect_true(Deserializer{"null"}.deserialize_null().is_ok()); }

  // bool
  {
    Str s[] = {"true", "false"};
    panicking::expect_eq(Deserializer{s[0]}.deserialize_bool().ok(), Option{true});
    panicking::expect_eq(Deserializer{s[1]}.deserialize_bool().ok(), Option{false});
  }

  // int
  {
    Str s[] = {"0", "123", "-123"};
    panicking::expect_eq(Deserializer{s[0]}.deserialize_int<int>().ok(), Option{0});
    panicking::expect_eq(Deserializer{s[1]}.deserialize_int<int>().ok(), Option{123});
    panicking::expect_eq(Deserializer{s[2]}.deserialize_int<int>().ok(), Option{-123});
  }

  // float
  {
    Str s[] = {"0.0", "1.23", "-1.23"};
    panicking::expect_eq(Deserializer{s[0]}.deserialize_flt<double>().ok(), Option{0.0});
    panicking::expect_eq(Deserializer{s[1]}.deserialize_flt<double>().ok(), Option{1.23});
    panicking::expect_eq(Deserializer{s[2]}.deserialize_flt<double>().ok(), Option{-1.23});
  }

  // str
  {
    Str s[] = {"\"\"", "\"abc\""};
    panicking::expect_eq(Deserializer{s[0]}.deserialize_string().ok(), Option<Str>{""});
    panicking::expect_eq(Deserializer{s[1]}.deserialize_string().ok(), Option<Str>{"abc"});
  }
}

SFC_TEST(deserialize_seq) {
  Str s[] = {R"([0,1,2])"};
  auto des = Deserializer{s[0]};
  auto seq = des.deserialize_seq();

  for (auto i = 0; i < 3; i++) {
    auto x_res = seq.next();
    panicking::expect(x_res.is_ok());

    auto& x = *x_res;
    auto val = x->extract_item<int>();
    panicking::expect_eq(val.is_ok(), true);
    panicking::expect_eq(*val, i);
  }

  const auto item_end = seq.next();
  panicking::expect(item_end.is_ok());
  panicking::expect(item_end->is_none());
}

SFC_TEST(deserialize_map) {
  Str s[] = {R"({"a":1,"b":2})"};
  auto des = Deserializer{s[0]};
  auto map = des.deserialize_map();

  Str keys[] = {"a", "b"};
  int vals[] = {1, 2};

  for (auto i = 0; i < 2; i++) {
    auto x_res = map.next();
    panicking::expect(x_res.is_ok());

    auto& x = *x_res;

    const auto key = x->extract_key();
    panicking::expect(key.is_ok());
    panicking::expect_eq(*key, keys[i]);

    const auto val = x->extract_val<int>();
    panicking::expect(val.is_ok());
    panicking::expect_eq(*val, vals[i]);
  }

  const auto status = map.next();
  panicking::expect(status.is_ok());
  panicking::expect(status->is_none());
}

}  // namespace sfc::serde::json::test
