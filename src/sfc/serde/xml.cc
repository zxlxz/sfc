#include "sfc/serde/xml.h"

namespace sfc::serde {

XmlNode::XmlNode(Str name, Str text) : _name{String::from(name)}, _text{String::from(text)} {}

XmlNode::~XmlNode() = default;

XmlNode::XmlNode(XmlNode&& other) noexcept = default;

XmlNode& XmlNode::operator=(XmlNode&& other) noexcept = default;

auto XmlNode::add_attr(Str name, Str value) -> XmlAttr& {
  return _attrs.push(XmlAttr{String::from(name), String::from(value)});
}

auto XmlNode::add_node(XmlNode child) -> XmlNode& {
  return _children.push(mem::move(child));
}

XmlDoc::XmlDoc(Str version, Str encoding)
    : _version{String::from(version)}, _encoding{String::from(encoding)} {}

XmlDoc::~XmlDoc() = default;

XmlDoc::XmlDoc(XmlDoc&& other) noexcept = default;

XmlDoc& XmlDoc::operator=(XmlDoc&& other) noexcept = default;

auto XmlDoc::add_node(XmlNode node) -> XmlNode& {
  _nodes.push(mem::move(node));
  return _nodes[_nodes.len() - 1];
}

}  // namespace sfc::serde
