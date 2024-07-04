#pragma once

#include "fmter.h"

namespace sfc::fmt {

template <class W>
class DebugTuple {
  ptr::Unique<W> _fmt;
  usize _cnt = 0;

 public:
  explicit DebugTuple(W& fmt) : _fmt{&fmt} {
    _fmt->write_str("(");
  }

  ~DebugTuple() {
    if (!_fmt) return;
    _fmt->write_str(")");
  }

  DebugTuple(DebugTuple&&) noexcept = default;

  void entry(const auto& val) {
    if (_cnt != 0) {
      _fmt->write_str(", ");
    }
    _fmt->write(val);
    _cnt += 1;
  }

  void entries(auto iter) {
    iter.for_each([&](auto&& val) { this->entrie(val); });
  }
};

template <class W>
class DebugList {
  ptr::Unique<W> _fmt;
  usize _cnt = 0;

 public:
  explicit DebugList(W& fmt) : _fmt{&fmt}, _cnt{0} {
    _fmt->write_str("[");
  }

  ~DebugList() {
    if (!_fmt) return;
    _fmt->write_str("]");
  }

  DebugList(DebugList&&) noexcept = default;

  void entry(const auto& val) {
    if (_cnt != 0) {
      _fmt->write_str(", ");
    }
    _fmt->write(val);
    _cnt += 1;
  }

  void entries(auto iter) {
    iter.for_each([&](auto&& val) { this->entry(val); });
  }
};

template <class W>
class DebugSet {
  ptr::Unique<W> _fmt;
  usize _cnt = 0;

 public:
  explicit DebugSet(W& fmt) : _fmt{&fmt}, _cnt{0} {
    _fmt->write_str("{");
  }

  ~DebugSet() {
    if (!_fmt) return;
    _fmt->write_str("}");
  }

  DebugSet(DebugSet&&) noexcept = default;

  void entry(const auto& val) {
    if (_cnt != 0) {
      _fmt->write_str(", ");
    }
    _fmt->write(val);
    _cnt += 1;
  }

  void entries(auto iter) {
    iter.for_each([&](auto&& val) { this->entrie(val); });
  }
};

template <class W>
class DebugMap {
  ptr::Unique<W> _fmt;
  usize _cnt = 0;

 public:
  explicit DebugMap(W& fmt) : _fmt{&fmt} {
    _fmt->write_str("{");
  }

  ~DebugMap() {
    if (!_fmt) return;
    _fmt->write_str("}");
  }

  DebugMap(DebugMap&&) noexcept = default;

  void entry(Str name, const auto& value) {
    if (_cnt != 0) {
      _fmt->write_str(", ");
    }

    _fmt->write_str("\"");
    _fmt->write_str(name);
    _fmt->write_str("\": ");
    _fmt->write(value);

    _cnt += 1;
  }

  void entries(auto iter) {
    iter.for_each([&](auto&& item) {  //
      this->entry(item.template get<0>(), item.template get<1>());
    });
  }
};

template <class W>
class DebugStruct {
  ptr::Unique<W> _fmt;
  usize _cnt = 0;

 public:
  explicit DebugStruct(W& fmt) : _fmt{&fmt} {
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

    _fmt->write_str(name);
    _fmt->write_str(": ");
    _fmt->write(value);

    _cnt += 1;
  }

  void fields(auto iter) {
    iter.for_each([&](auto&& item) {  //
      this->entry(item.template get<0>(), item.template get<1>());
    });
  }
};

template <class W>
inline auto Fmter<W>::debug_tuple() {
  return DebugTuple<Fmter<W>>{*this};
}

template <class W>
inline auto Fmter<W>::debug_list() {
  return DebugList<Fmter<W>>{*this};
}

template <class W>
inline auto Fmter<W>::debug_set() {
  return DebugSet<Fmter<W>>{*this};
}

template <class W>
inline auto Fmter<W>::debug_map() {
  return DebugMap<Fmter<W>>{*this};
}

template <class W>
inline auto Fmter<W>::debug_struct() {
  return DebugStruct<Fmter<W>>{*this};
}

}  // namespace sfc::fmt
