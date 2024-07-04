#include "sfc/serde/xml.h"

namespace sfc::serde {

XmlNode::XmlNode(Str type, Str value) : _type{String::from(type)}, _value{String::from(value)} {}

XmlNode::~XmlNode()noexcept = default;

XmlNode::XmlNode(XmlNode&& other) noexcept = default;

XmlNode& XmlNode::operator=(XmlNode&& other) noexcept = default;

auto XmlNode::add_attr(Str name, Str value) -> XmlAttr& {
  return _attrs.push(XmlAttr{String::from(name), String::from(value)});
}

auto XmlNode::add_node(XmlNode child) -> XmlNode& {
  return _children.push(mem::move(child));
}

auto XmlNode::to_string(u32 depth) const -> String {
  auto buf = String::with_capacity(256);

  auto indent = [&]() -> void {
    for (auto i = 0U; i < depth; ++i) {
      buf.push_str("    ");
    }
  };

  indent();
  buf.push('<');
  buf.push_str(_type.as_str());

  for (const auto& attr : _attrs) {
    buf.push(' ');
    buf.push_str(attr.name.as_str());
    buf.push_str("=\"");
    buf.push_str(attr.value.as_str());
    buf.push_str("\"");
  }
  if (_children.is_empty() && _value.is_empty()) {
    buf.push_str("/>");
    return buf;
  }
  buf.push('>');

  if (!_value.is_empty()) {
    buf.push_str(_value.as_str());
  }
  for (const auto& child : _children) {
    buf.push('\n');
    const auto child_str = child.to_string(depth + 1);
    buf.push_str(child_str.as_str());
  }

  if (!_children.is_empty()) {
    buf.push('\n');
    indent();
  }

  buf.push_str("</");
  buf.push_str(_type.as_str());
  buf.push_str(">");
  return buf;
}

}  // namespace sfc::serde
