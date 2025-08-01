#pragma once

#include "sfc/alloc.h"

namespace sfc::serde {

struct XmlAttr {
  String name;
  String value;
};

class XmlNode {
  String _name{};
  String _text{};
  Vec<XmlAttr> _attrs{};
  Vec<XmlNode> _children{};

 public:
  XmlNode(Str name, Str text = {});
  ~XmlNode();

  XmlNode(XmlNode&&) noexcept;
  XmlNode& operator=(XmlNode&&) noexcept;

  auto add_attr(Str name, Str value) -> XmlAttr&;
  auto add_node(XmlNode child) -> XmlNode&;
};

class XmlDoc {
  String _version{};
  String _encoding{};
  Vec<XmlNode> _nodes{};

 public:
  XmlDoc(Str version, Str encoding);
  ~XmlDoc();

  XmlDoc(XmlDoc&&) noexcept;
  XmlDoc& operator=(XmlDoc&&) noexcept;

  auto add_node(XmlNode node) -> XmlNode&;
};

}  // namespace sfc::serde
