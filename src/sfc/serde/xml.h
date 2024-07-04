#pragma once

#include "sfc/alloc.h"

namespace sfc::serde {

struct XmlAttr {
  String name;
  String value;
};

class XmlNode {
  String _type{};
  String _value{};
  Vec<XmlAttr> _attrs{};
  Vec<XmlNode> _children{};

 public:
  XmlNode(Str type, Str value = {});
  ~XmlNode()noexcept;

  XmlNode(XmlNode&&) noexcept;
  XmlNode& operator=(XmlNode&&) noexcept;

  auto add_attr(Str name, Str value) -> XmlAttr&;
  auto add_node(XmlNode child) -> XmlNode&;

  auto to_string(u32 depth = 0) const -> String;
};

}  // namespace sfc::serde
