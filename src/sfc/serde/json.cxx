

#include "sfc/serde.h"
#include "sfc/test.h"

namespace sfc::serde {

SFC_TEST(parse_null) {
  auto val = Node::from_json("null");
  panicking::expect_true(val);
  panicking::expect_true(val->is<Null>());
}

SFC_TEST(parse_bool) {
  {
    auto val = Node::from_json("true");
    panicking::expect_true(val);
    panicking::expect_true(val->is<bool>());
    panicking::expect_eq(val->as<bool>(), true);
  }

  {
    auto val = Node::from_json("false");
    panicking::expect_true(val);
    panicking::expect_true(val->is<bool>());
    panicking::expect_eq(val->as<bool>(), false);
  }
}

SFC_TEST(parse_int) {
  {
    auto val = Node::from_json("123");
    panicking::expect_true(val);
    panicking::expect_true(val->is<i64>());
    panicking::expect_eq(val->as<i64>(), 123);
  }

  {
    auto val = Node::from_json("-123");
    panicking::expect_true(val);
    panicking::expect_true(val->is<i64>());
    panicking::expect_eq(val->as<i64>(), -123);
  }
}

SFC_TEST(parse_flt) {
  {
    auto val = Node::from_json("1.23");
    panicking::expect_true(val);
    panicking::expect_true(val->is<f64>());
    panicking::expect_eq(val->as<f64>(), 1.23);
  }

  {
    auto val = Node::from_json("-1.23");
    panicking::expect_true(val);
    panicking::expect_true(val->is<f64>());
    panicking::expect_flt_eq(val->as<f64>(), -1.23);
  }
}

SFC_TEST(parse_str) {
  {
    auto val = Node::from_json(R"("abc")");
    panicking::expect_true(val);
    panicking::expect_true(val->is<String>());
    panicking::expect_eq(val->as<String>(), "abc");
  }
}

SFC_TEST(parse_list) {
  {
    auto opt = Node::from_json("[]");
    panicking::expect_true(opt);

    const auto& val = *opt;
    panicking::expect_true(val.is<List>());
    panicking::expect_eq(val.as<List>().len(), 0U);
  }

  {
    auto val = Node::from_json("[ ]");
    panicking::expect_true(val);

    panicking::expect_true(val->is<List>());
    panicking::expect_eq(val->as<List>().len(), 0U);
  }

  {
    auto val = Node::from_json("[\n]");
    panicking::expect_true(val);

    panicking::expect_true(val->is<List>());
    panicking::expect_eq(val->as<List>().len(), 0U);
  }

  {
    auto val = Node::from_json("[ [] , [] ]");
    panicking::expect_true(val);
    panicking::expect_true(val->is<List>());
    panicking::expect_true((*val)[0].is<List>());
    panicking::expect_true((*val)[1].is<List>());
  }

  {
    auto val = Node::from_json("[ [\n] , [\t[ ]\n] ]");
    panicking::expect_true(val);

    panicking::expect_true(val->is<List>());
    panicking::expect_true((*val)[0].is<List>());
    panicking::expect_true((*val)[1].is<List>());
    panicking::expect_true((*val)[1][0].is<List>());
  }

  {
    auto val = Node::from_json("[ 1 ]");
    panicking::expect_true(val);

    panicking::expect_true(val->is<List>());
    panicking::expect_eq(val->as<List>().len(), 1U);
  }

  {
    auto val = Node::from_json("[ \n1 \n ]");
    panicking::expect_true(val);

    panicking::expect_true(val->is<List>());
    panicking::expect_eq(val->as<List>().len(), 1U);
  }

  {
    auto val = Node::from_json("\n[ \n1, \n 2 \n ]");
    panicking::expect_true(val);
    panicking::expect_true(val->is<List>());
    panicking::expect_eq(val->as<List>().len(), 2U);
  }
}

SFC_TEST(parse_dict) {
  {
    auto val = Node::from_json("{}");
    panicking::expect_true(val);
    panicking::expect_true(val->is<Dict>());
    auto& dict = val->as<Dict>();
    panicking::expect_eq(dict.len(), 0U);
  }

  {
    auto val = Node::from_json("{  }");
    panicking::expect_true(val);
    panicking::expect_true(val->is<Dict>());
    auto& dict = val->as<Dict>();
    panicking::expect_eq(dict.len(), 0U);
  }

  {
    auto val = Node::from_json("{ \n }");
    panicking::expect_true(val);
    panicking::expect_true(val->is<Dict>());
    auto& dict = val->as<Dict>();
    panicking::expect_eq(dict.len(), 0U);
  }

  {
    auto val = Node::from_json(R"({ "a": 1 })");
    panicking::expect_true(val);
    panicking::expect_true(val->is<Dict>());
    auto& dict = val->as<Dict>();
    panicking::expect_eq(dict.len(), 1U);
    panicking::expect_eq(dict["a"].as<i64>(), 1);
  }

  {
    auto val = Node::from_json(R"({"a":1, "b" : 2})");
    panicking::expect_true(val);
    panicking::expect_true(val->is<Dict>());
    auto& dict = val->as<Dict>();
    panicking::expect_eq(dict["a"].as<i64>(), 1);
    panicking::expect_eq(dict["b"].as<i64>(), 2);
  }
}

}  // namespace sfc::serde
