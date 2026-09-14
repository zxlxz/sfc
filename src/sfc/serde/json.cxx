

#include "sfc/test/test.h"
#include "sfc/collections.h"
#include "sfc/serde/json.h"

namespace sfc::serde::json::test {

SFC_TEST(serde_bool) {
  // true
  {
    sfc::assert_eq(json::to_string(true), "true");

    auto buf = Str{"true"}.as_bytes();
    auto des = Deserializer{buf};
    sfc::assert_eq(des.deserialize_bool().ok(), Option{true});
  }

  // false
  {
    sfc::assert_eq(json::to_string(false), "false");

    auto buf = Str{"false"}.as_bytes();
    auto des = Deserializer{buf};
    sfc::assert_eq(des.deserialize_bool().ok(), Option{false});
  }
}

SFC_TEST(serde_int) {
  // 123
  {
    sfc::assert_eq(json::to_string(123), "123");

    auto buf = Str{"123"}.as_bytes();
    auto des = Deserializer{buf};
    sfc::assert_eq(des.deserialize_i64().ok(), Option{123});
  }

  // -123 -> i64
  {
    sfc::assert_eq(json::to_string(-123), "-123");

    auto buf = Str{"-123"}.as_bytes();
    auto des = Deserializer{buf};
    sfc::assert_eq(des.deserialize_i64().ok(), Option{-123});
  }

  // -123 -> u64
  {
    sfc::assert_eq(json::to_string(-123), "-123");

    auto buf = Str{"-123"}.as_bytes();
    auto des = Deserializer{buf};
    sfc::assert_eq(des.deserialize_u64().ok(), Option<u64>{});
  }
}

SFC_TEST(serde_flt) {
  // 1.2
  {
    sfc::assert_eq(json::to_string(1.2), "1.200000");

    auto buf = Str{"1.2"}.as_bytes();
    auto des = Deserializer{buf};
    sfc::assert_eq(des.deserialize_f64().ok(), Option{1.2});
  }

  // -1.2
  {
    sfc::assert_eq(json::to_string(-1.2), "-1.200000");

    auto buf = Str{"-1.2"}.as_bytes();
    auto des = Deserializer{buf};
    sfc::assert_eq(des.deserialize_f64().ok(), Option{-1.2});
  }
}

SFC_TEST(serde_str) {
  // abc
  {
    sfc::assert_eq(json::to_string("abc"), "\"abc\"");

    auto buf = Str{"\"abc\""}.as_bytes();
    auto des = Deserializer{buf};
    sfc::assert_eq(des.deserialize_string().ok(), Option{Str{"abc"}});
  }

  // a b c
  {
    sfc::assert_eq(json::to_string("a b c"), "\"a b c\"");

    auto buf = Str{"\"a b c\""}.as_bytes();
    auto des = Deserializer{buf};
    sfc::assert_eq(des.deserialize_string().ok(), Option{Str{"a b c"}});
  }
}

SFC_TEST(serde_seq) {
  // [0, 1, 2]
  {
    const int vals[] = {0, 1, 2};
    sfc::assert_eq(json::to_string(vals), "[0,1,2]");

    auto buf = Str{"[0,1,2]"}.as_bytes();
    auto des = Deserializer{buf};
    auto ret = des.deserialize_seq([&](DeserializeSeq& seq) -> Result<> {
      for (auto i = 0U; i < 3; ++i) {
        const auto val = _TRY(seq.next_element<int>());
        sfc::assert_eq(val, Option{vals[i]});
      }
      return Ok{};
    });
    sfc::assert_eq(ret.is_ok(), true);
  }
}

SFC_TEST(serde_map) {
  // {"a":1,"b":2}
  {
    auto dict = collections::HashMap<Str, int>{};
    dict.insert(Str{"a"}, 1);
    dict.insert(Str{"b"}, 2);
    sfc::assert_eq(json::to_string(dict), "{\"a\":1,\"b\":2}");

    auto buf = Str{"{\"a\":1,\"b\":2}"}.as_bytes();
    auto des = Deserializer{buf};

    const Str keys[] = {Str{"a"}, Str{"b"}};
    const int vals[] = {1, 2};
    auto ret = des.deserialize_obj([&](DeserializeObj& map) -> Result<> {
      for (auto i = 0U; i < 2; ++i) {
        const auto key = _TRY(map.next_key());
        sfc::assert_eq(key, Option{keys[i]});

        const auto val = _TRY(map.next_val<int>());
        sfc::assert_eq(val, vals[i]);
      }
      return Ok{};
    });
    sfc::assert_eq(ret.is_ok(), true);
  }
}

}  // namespace sfc::serde::json::test
