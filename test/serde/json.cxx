

#include "sfc/serde.h"
#include "sfc/test.h"

namespace sfc::serde::json {

SFC_TEST(parse_null) {
  auto val = json::parse("null").unwrap();
  test::assert_true(val.is_null());
}

SFC_TEST(parse_bool) {
  {
    auto val = json::parse("true").unwrap();
    test::assert_true(val.is_bool());
    test::assert_eq(val.as_bool(), true);
  }

  {
    auto val = json::parse("false").unwrap();
    test::assert_true(val.is_bool());
    test::assert_eq(val.as_bool(), false);
  }
}

SFC_TEST(parse_int) {
  {
    auto val = json::parse("123").unwrap();
    test::assert_true(val.is_int());
    test::assert_eq(val.as_int(), 123);
  }

  {
    auto val = json::parse("-123").unwrap();
    test::assert_true(val.is_int());
    test::assert_eq(val.as_int(), -123);
  }
}

SFC_TEST(parse_flt) {
  {
    auto val = json::parse("1.23").unwrap();
    test::assert_true(val.is_flt());
    test::assert_eq(val.as_flt(), 1.23);
  }

  {
    auto val = json::parse("-1.23").unwrap();
    test::assert_true(val.is_flt());
    test::assert_eq(val.as_flt(), -1.23);
  }
}

SFC_TEST(parse_str) {
  {
    auto val = json::parse(R"("abc")").unwrap();
    test::assert_true(val.is_str());
    test::assert_eq(val.as_str(), "abc");
  }
}

SFC_TEST(parse_list) {
  {
    auto opt = json::parse("[]");
    test::assert_true(opt);

    const auto& val = *opt;
    test::assert_true(val.is_list());
    test::assert_eq(val.as_list().len(), 0U);
  }

  {
    auto opt = json::parse("[ ]");
    test::assert_true(opt);

    auto& val = *opt;
    test::assert_true(val.is_list());
    test::assert_eq(val.as_list().len(), 0U);
  }

  {
    auto opt = json::parse("[\n]");
    test::assert_true(opt);

    auto& val = *opt;
    test::assert_true(val.is_list());
    test::assert_eq(val.as_list().len(), 0U);
  }

  {
    auto opt = json::parse("[ [] , [] ]");
    test::assert_true(opt);

    auto& val = *opt;

    test::assert_true(val.is_list());
    test::assert_true(val[0].is_list());
    test::assert_true(val[1].is_list());
  }

  {
    auto opt = json::parse("[ [\n] , [\t[ ]\n] ]");
    test::assert_true(opt);

    auto& val = *opt;

    test::assert_true(val.is_list());
    test::assert_true(val[0].is_list());
    test::assert_true(val[1].is_list());
    test::assert_true(val[1][0].is_list());
  }

  {
    auto opt = json::parse("[ 1 ]");
    test::assert_true(opt);

    auto& val = *opt;
    test::assert_true(val.is_list());
    test::assert_eq(val.as_list().len(), 1U);
  }

  {
    auto opt = json::parse("[ \n1 \n ]");
    test::assert_true(opt);

    auto& val = *opt;
    test::assert_true(val.is_list());
    test::assert_eq(val.as_list().len(), 1U);
  }

  {
    auto opt = json::parse("\n[ \n1, \n 2 \n ]");
    test::assert_true(opt);
    auto& val = *opt;

    test::assert_true(val.is_list());
    test::assert_eq(val.as_list().len(), 2U);
  }
}

SFC_TEST(parse_dict) {
  {
    auto val = json::parse("{}").unwrap();
    test::assert_true(val.is_dict());
    test::assert_eq(val.as_dict().len(), 0U);
  }

  {
    auto val = json::parse("{  }").unwrap();
    test::assert_true(val.is_dict());
    test::assert_eq(val.as_dict().len(), 0U);
  }

  {
    auto val = json::parse("{ \n }").unwrap();
    test::assert_true(val.is_dict());
    test::assert_eq(val.as_dict().len(), 0U);
  }

  {
    auto val = json::parse(R"({ "a": 1 })").unwrap();
    test::assert_true(val.is_dict());
    test::assert_eq(val.as_dict().len(), 1U);
    test::assert_eq(val.as_dict()["a"].as_int(), 1);
  }

  {
    auto val = json::parse(R"({"a":1, "b" : 2})").unwrap();
    test::assert_true(val.is_dict());
    test::assert_eq(val.as_dict()["a"].as_int(), 1);
    test::assert_eq(val.as_dict()["b"].as_int(), 2);
  }
}

}  // namespace sfc::serde::json
