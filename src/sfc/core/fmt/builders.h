#pragma once

#include "sfc/core/fmt/fmter.h"

namespace sfc::fmt {

template <class W>
class DebugTuple {
  W&    _fmt;
  usize _cnt = 0;

 public:
  explicit DebugTuple(W& fmt) : _fmt{fmt} {
    _fmt.write_str("(");
  }

  ~DebugTuple() {
    _fmt.write_str(")");
  }

  DebugTuple(const DebugTuple&) = delete;

  void entry(const auto& val) {
    if (_cnt != 0) {
      _fmt.write_str(", ");
    }
    _fmt.write(val);
    _cnt += 1;
  }

  void entries(auto&& iter) {
    iter.for_each([&](auto&& val) { this->entry(val); });
  }
};

template <class W>
class DebugList {
  W&    _fmt;
  usize _cnt = 0;

 public:
  explicit DebugList(W& fmt) : _fmt{fmt} {
    _fmt.write_str("[");
  }

  ~DebugList() {
    _fmt.write_str("]");
  }

  DebugList(const DebugList&) noexcept = delete;

  void entry(const auto& val) {
    if (_cnt != 0) {
      _fmt.write_str(", ");
    }
    _fmt.write(val);
    _cnt += 1;
  }

  void entries(auto&& iter) {
    iter->for_each([&](auto&& val) { this->entry(val); });
  }
};

template <class W>
class DebugSet {
  W&    _fmt;
  usize _cnt = 0;

 public:
  explicit DebugSet(W& fmt) : _fmt{fmt} {
    _fmt.write_str("{");
  }

  ~DebugSet() {
    if (!_fmt) {
      return;
    }
    _fmt.write_str("}");
  }

  DebugSet(const DebugSet&) noexcept = delete;

  void entry(const auto& val) {
    if (_cnt != 0) {
      _fmt.write_str(", ");
    }
    _fmt.write(val);
    _cnt += 1;
  }

  void entries(auto&& iter) {
    iter.for_each([&](auto&& val) { this->entry(val); });
  }
};

template <class W>
class DebugMap {
  W&    _fmt;
  usize _cnt = 0;

 public:
  explicit DebugMap(W& fmt) : _fmt{fmt} {
    _fmt.write_str("{");
  }

  ~DebugMap() {
    _fmt.write_str("}");
  }

  DebugMap(const DebugMap&) noexcept = delete;

  void entry(const auto& name, const auto& value) {
    if (_cnt != 0) {
      _fmt.write_str(", ");
    }

    _fmt.write_str("\"");
    _fmt.write_str(name);
    _fmt.write_str("\": ");
    _fmt.write(value);

    _cnt += 1;
  }

  void entries(auto iter) {
    iter->for_each([&](auto&& item) {
      const auto& [k, v] = item;
      this->entry(k, v);
    });
  }
};

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

  DebugStruct(DebugStruct&&) noexcept = delete;

  void field(str::Str name, const auto& value) {
    if (_cnt != 0) {
      _fmt.write_str(", ");
    }

    _fmt.write_str(name);
    _fmt.write_str(": ");
    _fmt.write(value);

    _cnt += 1;
  }

  void fields(auto iter) {
    iter.for_each([&](auto&& item) {
      const auto& [k, v] = item;
      this->field(k, v);
    });
  }
};

template <class W>
inline auto Fmter<W>::debug_tuple() {
  return DebugTuple<Fmter<W>>{*this};
}

template <class W>
inline auto Fmter<W>::debug_list() {
  return DebugList<Fmter>{*this};
}

template <class W>
inline auto Fmter<W>::debug_set() {
  return DebugSet<Fmter>{*this};
}

template <class W>
inline auto Fmter<W>::debug_map() {
  return DebugMap<Fmter>{*this};
}

template <class W>
inline auto Fmter<W>::debug_struct() {
  return DebugStruct<Fmter>{*this};
}

}  // namespace sfc::fmt
