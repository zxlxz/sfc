

#include "sfc/serde.h"
#include "sfc/test.h"

namespace sfc::serde::json {

SFC_TEST(parse_null) {
  auto val = json::parse("null");
  panicking::assert_true(val);
  panicking::assert_true(val->is_null());
}

SFC_TEST(parse_bool) {
  {
    auto val = json::parse("true");
    panicking::assert_true(val);
    panicking::assert_true(val->is_bool());
    panicking::assert_eq(val->as_bool(), true);
  }

  {
    auto val = json::parse("false");
    panicking::assert_true(val);
    panicking::assert_true(val->is_bool());
    panicking::assert_eq(val->as_bool(), false);
  }
}

SFC_TEST(parse_int) {
  {
    auto val = json::parse("123");
    panicking::assert_true(val);
    panicking::assert_true(val->is_int());
    panicking::assert_eq(val->as_int(), 123);
  }

  {
    auto val = json::parse("-123");
    panicking::assert_true(val);
    panicking::assert_true(val->is_int());
    panicking::assert_eq(val->as_int(), -123);
  }
}

SFC_TEST(parse_flt) {
  {
    auto val = json::parse("1.23");
    panicking::assert_true(val);
    panicking::assert_true(val->is_flt());
    panicking::assert_eq(val->as_flt(), 1.23);
  }

  {
    auto val = json::parse("-1.23");
    panicking::assert_true(val);
    panicking::assert_true(val->is_flt());
    panicking::assert_flt_eq(val->as_flt(), -1.23);
  }
}

SFC_TEST(parse_str) {
  {
    auto val = json::parse(R"("abc")");
    panicking::assert_true(val);
    panicking::assert_true(val->is_str());
    panicking::assert_eq(val->as_str(), "abc");
  }
}

SFC_TEST(parse_list) {
  {
    auto opt = json::parse("[]");
    panicking::assert_true(opt);

    const auto& val = *opt;
    panicking::assert_true(val.is_list());
    panicking::assert_eq(val.as_list().len(), 0U);
  }

  {
    auto val = json::parse("[ ]");
    panicking::assert_true(val);

    panicking::assert_true(val->is_list());
    panicking::assert_eq(val->as_list().len(), 0U);
  }

  {
    auto val = json::parse("[\n]");
    panicking::assert_true(val);

    panicking::assert_true(val->is_list());
    panicking::assert_eq(val->as_list().len(), 0U);
  }

  {
    auto val = json::parse("[ [] , [] ]");
    panicking::assert_true(val);
    panicking::assert_true(val->is_list());
    panicking::assert_true((*val)[0].is_list());
    panicking::assert_true((*val)[1].is_list());
  }

  {
    auto val = json::parse("[ [\n] , [\t[ ]\n] ]");
    panicking::assert_true(val);

    panicking::assert_true(val->is_list());
    panicking::assert_true((*val)[0].is_list());
    panicking::assert_true((*val)[1].is_list());
    panicking::assert_true((*val)[1][0].is_list());
  }

  {
    auto val = json::parse("[ 1 ]");
    panicking::assert_true(val);

    panicking::assert_true(val->is_list());
    panicking::assert_eq(val->as_list().len(), 1U);
  }

  {
    auto val = json::parse("[ \n1 \n ]");
    panicking::assert_true(val);

    panicking::assert_true(val->is_list());
    panicking::assert_eq(val->as_list().len(), 1U);
  }

  {
    auto val = json::parse("\n[ \n1, \n 2 \n ]");
    panicking::assert_true(val);
    panicking::assert_true(val->is_list());
    panicking::assert_eq(val->as_list().len(), 2U);
  }
}

SFC_TEST(parse_dict) {
  {
    auto val = json::parse("{}");
    panicking::assert_true(val);
    panicking::assert_true(val->is_dict());
    panicking::assert_eq(val->as_dict().len(), 0U);
  }

  {
    auto val = json::parse("{  }");
    panicking::assert_true(val);
    panicking::assert_true(val->is_dict());
    panicking::assert_eq(val->as_dict().len(), 0U);
  }

  {
    auto val = json::parse("{ \n }");
    panicking::assert_true(val);
    panicking::assert_true(val->is_dict());
    panicking::assert_eq(val->as_dict().len(), 0U);
  }

  {
    auto val = json::parse(R"({ "a": 1 })");
    panicking::assert_true(val);
    panicking::assert_true(val->is_dict());
    panicking::assert_eq(val->as_dict().len(), 1U);
    panicking::assert_eq(val->as_dict()["a"].as_int(), 1);
  }

  {
    auto val = json::parse(R"({"a":1, "b" : 2})");
    panicking::assert_true(val);
    panicking::assert_true(val->is_dict());
    panicking::assert_eq(val->as_dict()["a"].as_int(), 1);
    panicking::assert_eq(val->as_dict()["b"].as_int(), 2);
  }
}

}  // namespace sfc::serde::json
