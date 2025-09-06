#pragma once

#include "sfc/alloc.h"

namespace sfc::serde {

struct XmlAttr {
  String name;
  String value;

 public:
  template <class Name, class Value>
  static auto from(Name&& name, Value&& value) -> XmlAttr {
    return XmlAttr{
        String::from(static_cast<Name&&>(name)),
        String::from(static_cast<Value&&>(value)),
    };
  }
};

class XmlNode {
  String _type{};
  String _value{};
  Vec<XmlAttr> _attrs{};
  Vec<XmlNode> _children{};

 public:
  explicit XmlNode(Str type, Str value = {});
  ~XmlNode() noexcept;

  XmlNode(XmlNode&&) noexcept;
  XmlNode& operator=(XmlNode&&) noexcept;

  auto add_attr(XmlAttr attr) -> XmlAttr&;
  auto add_node(XmlNode child) -> XmlNode&;

  auto to_string(u32 depth = 0) const -> String;
};

}  // namespace sfc::serde
