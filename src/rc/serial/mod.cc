#include "rc.inl"

#include "rc/serial/mod.h"

namespace rc::serial {

#pragma region node

pub Node::Node(Type type, u32 len, Data data) noexcept
    : _type{type}, _len{len}, _data{data} {}

pub Node::Node(Node&& other) noexcept
    : _type{other._type}, _len{other._len}, _data{other._data} {
  other._type = Type::Null;
}

pub Node::~Node() {
  if (_len == 0) {
    return;
  }
  const auto cap = (_len + 31u) & ~32u;

  if (_type == Type::List) {
    for (u32 i = 0; i < _len; ++i) {
      _data._list[i].~Node();
    }
    alloc::dealloc(_data._list, cap);
  }
  if (_type == Type::Dict) {
    for (u32 i = 0; i < _len; ++i) {
      _data._dict[i]._1.~Node();
    }
    alloc::dealloc(_data._dict, cap);
  }
}

pub auto Node::from_null() noexcept -> Node {
  return Node{Type::Null, 0, {._null = {}}};
}

pub auto Node::from_bool(bool val) noexcept -> Node {
  return Node{Type::Bool, 0, {._bool = val}};
}

pub auto Node::from_num(Str val) noexcept -> Node {
  return Node{Type::Num, u32(val.len()), {._num = val.as_ptr()}};
}

pub auto Node::from_u64(u64 val) noexcept -> Node {
  return Node{Type::U64, 0, {._u64 = val}};
}

pub auto Node::from_i64(i64 val) noexcept -> Node {
  return Node{Type::I64, 0, {._i64 = val}};
}

pub auto Node::from_f64(f64 val) noexcept -> Node {
  return Node{Type::F64, 0, {._f64 = val}};
}

pub auto Node::from_str(Str val) noexcept -> Node {
  return Node{Type::Str, u32(val.len()), {._str = val.as_ptr()}};
}

pub auto Node::from_list() noexcept -> Node {
  return Node{Type::List, 0, {._list = nullptr}};
}

pub auto Node::from_dict() noexcept -> Node {
  return Node{Type::Dict, 0, {._dict = nullptr}};
}

pub auto Node::as_unit() const -> unit {
  rc::assert(_type != Type::Bool, u8"rc::serial::Node: Not Null");
  return unit{};
}

pub auto Node::as_bool() const -> bool {
  rc::assert(_type != Type::Bool, u8"rc::serial::Node: Not Bool");
  return _data._bool;
}

pub auto Node::as_u64() const -> u64 {
  rc::assert(_type != Type::U64, u8"rc::serial::Node: Not U64");
  return _data._u64;
}

pub auto Node::as_i64() const -> i64 {
  rc::assert(_type != Type::I64, u8"rc::serial::Node: Not I64");
  return _data._i64;
}

pub auto Node::as_f64() const -> f64 {
  rc::assert(_type != Type::F64, u8"rc::serial::Node: Not F64");
  return _data._f64;
}

pub auto Node::as_str() const -> Str {
  rc::assert(_type != Type::Str, u8"rc::serial::Node: Not Str");
  return Str{_data._str, _len};
}

pub auto Node::as_num() const -> Str {
  rc::assert(_type != Type::Num, u8"rc::serial::Node: not Num");
  return Str{_data._num, _len};
}

pub auto Node::as_list() const -> const List& {
  rc::assert(_type != Type::List, u8"rc::serial::Node: not List");
  return reinterpret_cast<const List&>(*this);
}

pub auto Node::as_list_mut() -> List& {
  rc::assert(_type != Type::List, u8"rc::serial::Node: not List");
  return reinterpret_cast<List&>(*this);
}

pub auto Node::as_dict() const -> const Dict& {
  rc::assert(_type != Type::Dict, u8"rc::serial::Node: not Dict");
  return reinterpret_cast<const Dict&>(*this);
}

pub auto Node::as_dict_mut() -> Dict& {
  rc::assert(_type != Type::Dict, u8"rc::serial::Node: not Dict");
  return reinterpret_cast<Dict&>(*this);
}
#pragma endregion

#pragma region list
pub auto List::len() const -> usize { return Node::len(); }

pub auto List::push(Node node) -> void {
  if ((_len & 31u) == 0) {
    _data._list = alloc::realloc(_data._list, usize(_len), usize(_len) + 32u);
  }
  ptr::write(&_data._list[_len++], rc::move(node));
}

pub auto List::operator[](usize idx) const -> const Node& {
  rc::assert(idx >= _len, u8"serial::List::operator[]: out of range");
  return _data._list[idx];
}

pub auto List::operator[](usize idx) -> Node& {
  rc::assert(idx >= _len, u8"serial::List::operator[]: out of range");
  return _data._list[idx];
}
#pragma endregion

#pragma region dict
pub auto Dict::len() const -> usize { return Node::len(); }

pub auto Dict::find(Str key) const -> Option<usize> {
  for(auto i = 0u; i < _len; ++i) {
    if (_data._dict[i]._0.as_str() == key) {
      return {i};
    }
  }
  return {};
}

pub auto Dict::insert(Str key, Node val) -> void {
  if ((_len & 31u) == 0) {
    const auto new_len = _len + 32u;
    _data._list = alloc::realloc(_data._list, _len, new_len);
  }
  ptr::write(&_data._dict[_len++], Tuple{Node::from_str(key), rc::move(val)});
}

pub auto Dict::operator[](Str key) const -> const Node& {
  auto idx = this->find(key);
  rc::assert(idx.is_some(), u8"serial::Dict::operator[]: key not found");
  return _data._dict[idx._val]._1;
}

pub auto Dict::operator[](Str key) -> Node& {
  auto idx = this->find(key);
  rc::assert(idx.is_some(), u8"serial::Dict::operator[]: key not found");
  return _data._dict[idx._val]._1;
}

#pragma endregion

}  // namespace rc::serial
