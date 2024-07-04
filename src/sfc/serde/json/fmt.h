#pragma once

#include "sfc/serde/mod.h"

namespace sfc::serde::json {

template <class W>
class Fmter : fmt::Fmter<W> {
  using Imp = fmt::Fmter<W>;
  using Imp::_style;

  class DebugStruct {
    ptr::Unique<Fmter> _fmt;
    usize _cnt = 0;

   public:
    explicit DebugStruct(Fmter& fmt) : _fmt{&fmt} {
      _fmt->write_str("{");
    }

    ~DebugStruct() {
      if (!_fmt) return;
      _fmt->write_str("}");
    }

    DebugStruct(DebugStruct&&) noexcept = default;

    void field(Str name, const auto& value) {
      if (_cnt != 0) {
        _fmt->write_str(", ");
      }

      _fmt->write_str("\"");
      _fmt->write_str(name);
      _fmt->write_str("\": ");
      _fmt->write(value);

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

  template <class... U>
  void write_fmt(Str fmts, const U&... args) {
    fmt::Args<U...>{fmts, args...}.fmt(*this);
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
  template <class T>
  void write_imp(const T& val) {
    trait::as<fmt::IFmt>(val).fmt(*this);
  }

  template <fmt::XFmt T>
  void write_imp(const T& val) {
    val.fmt(*this);
  }
};

auto format(const auto& val) -> String {
  auto res = String{};
  auto imp = Fmter<String>{res};
  imp.write(val);
  return res;
}

}  // namespace sfc::serde::json
