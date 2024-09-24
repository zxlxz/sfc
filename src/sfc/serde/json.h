#pragma once

#include "sfc/fs/path.h"
#include "sfc/serde/node.h"

namespace sfc::serde::json {

template <class W>
class Fmter : fmt::Fmter<W> {
  using Imp = fmt::Fmter<W>;

  class DebugStruct {
    Fmter& _fmt;
    usize _cnt = 0;

   public:
    explicit DebugStruct(Fmter& fmt) : _fmt{fmt} {
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
      iter.for_each([&](auto&& item) {
        const auto& k = item.template get<0>();
        const auto& v = item.template get<1>();
        this->entry(k, v);
      });
    }
  };

 public:
  using Imp::Imp;

  using Imp::set_style;
  using Imp::style;

  using Imp::pad_num;
  using Imp::write_chr;
  using Imp::write_str;

  void pad(Str val) {
    this->write_chr('"');
    this->write_str(val);
    this->write_chr('"');
  }

  void write(const auto& val) {
    return this->write_imp(val);
  }

  void write_fmt(const auto& fmts, const auto&... args) {
    fmt::Args{fmts, args...}.fmt(*this);
  }

  auto debug_tuple() {
    return fmt::DebugList<Fmter>{*this};
  }

  auto debug_list() {
    return fmt::DebugList<Fmter>{*this};
  }

  auto debug_set() {
    return fmt::DebugList<Fmter>{*this};
  }

  auto debug_map() {
    return fmt::DebugMap<Fmter>{*this};
  }

  auto debug_struct() {
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

auto load(fs::Path path) -> Option<Node>;

}  // namespace sfc::serde::json

namespace sfc::serde {

inline auto Node::from_json(Str text) -> Option<Node> {
  return json::parse(text);
}

inline auto Node::to_json() const -> String {
  return json::format(*this);
}

}  // namespace sfc::serde
