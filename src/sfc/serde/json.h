#pragma once

#include "sfc/fs/path.h"
#include "sfc/serde/node.h"

namespace sfc::serde::json {

using fmt::DebugList;
using fmt::DebugMap;
using fmt::DebugSet;
using fmt::DebugTuple;

template <class W>
class DebugStruct {
  W&    _fmt;
  usize _cnt = 0;

 public:
  explicit DebugStruct(W& fmt) : _fmt{fmt} {
    _fmt.write_str("{");
  }

  ~DebugStruct() {
    _fmt.write_str("}");
  }

  DebugStruct(const DebugStruct&) = delete;

  void field(Str name, const auto& value) {
    if (_cnt != 0) {
      _fmt.write_str(", ");
    }

    _fmt.write_str("\"");
    _fmt.write_str(name);
    _fmt.write_str("\": ");
    _fmt.write(value);

    _cnt += 1;
  }

  void fields(auto iter) {
    iter.for_each([&](const auto& item) {
      const auto& [k, v] = item;
      this->field(k, v);
    });
  }
};

template <class W>
class Fmter : fmt::Fmter<W> {
  using Imp = fmt::Fmter<W>;

 public:
  using Imp::Imp;
  using Imp::pad;
  using Imp::pad_num;
  using Imp::style;
  using Imp::write_str;

  void write(const auto& val) {
    return this->write_imp(val);
  }

  void write_fmt(const auto& fmts, const auto&... args) {
    fmt::Args{fmts, args...}.fmt(*this);
  }

  auto debug_tuple() -> DebugTuple<Fmter> {
    return {*this};
  }

  auto debug_list() -> DebugList<Fmter> {
    return DebugList{*this};
  }

  auto debug_set() -> DebugSet<Fmter> {
    return DebugSet{*this};
  }

  auto debug_map() -> DebugMap<Fmter> {
    return DebugMap{*this};
  }

  auto debug_struct() -> DebugStruct<Fmter> {
    return DebugStruct{*this};
  }

 private:
  void write_imp(const auto& val) {
    if constexpr (requires { val.fmt(*this); }) {
      val.fmt(*this);
    } else {
      trait::as<fmt::IFmt>(val).fmt(*this);
    }
  }
};

void write(auto& out, const auto& val) {
  Fmter{out}.write(val);
}

auto format(const auto& val) -> String {
  auto res = String{};
  Fmter<String>{res}.write(val);
  return res;
}

auto parse(Str json_str) -> Option<Node>;

}  // namespace sfc::serde::json

namespace sfc::serde {

inline auto Node::from_json(Str text) -> Option<Node> {
  return json::parse(text);
}

inline auto Node::to_json() const -> String {
  return json::format(*this);
}

}  // namespace sfc::serde
