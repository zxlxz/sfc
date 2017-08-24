#include <nms/serialization/json.h>
#include <nms/io/log.h>
#include <nms/io/console.h>
#include <nms/test.h>

namespace nms::serialization::xml
{

// format
void formatNode(String& buf, const NodeEx& node, i32 level = 0) {
    static const auto $indent = 2;
    buf.appends((level + 0)*$indent, ' ');

    StrView fmt(nullptr, { 0 });

    auto& v = node.val();

    switch (v.type()) {
    case Type::null:    formatImpl(buf, fmt, "");           break;
    case Type::boolean: formatImpl(buf, fmt, v.bool_val_);  break;
    case Type::u16:     formatImpl(buf, fmt, v.u16_val_);   break;
    case Type::i16:     formatImpl(buf, fmt, v.i16_val_);   break;
    case Type::u32:     formatImpl(buf, fmt, v.u32_val_);   break;
    case Type::i32:     formatImpl(buf, fmt, v.i32_val_);   break;
    case Type::u64:     formatImpl(buf, fmt, v.u64_val_);   break;
    case Type::i64:     formatImpl(buf, fmt, v.i64_val_);   break;
    case Type::f32:     formatImpl(buf, fmt, v.f32_val_);   break;
    case Type::f64:     formatImpl(buf, fmt, v.f64_val_);   break;

    case Type::number: {
        formatImpl(buf, fmt, v.str());
        break;
    }

    case Type::key: case Type::string: {
        buf += "\"";
        formatImpl(buf, fmt, v.str());
        buf += "\"";
        break;
    }

    case Type::array: {
        auto k = 0u;
        for (auto itr = node.begin(); itr != node.end(); ++k, ++itr) {
            if (k != 0) {
                buf.appends((level + 0) * $indent, ' ');
            }

            const auto val = *itr;
            if (val.type() == Type::object || val.type() == Type::array) {
                sformat(buf, "<{} type=\"{}\">\n", k, val.type());
                formatNode(buf, val, level + 1);
                buf += "\n";
                buf.appends((level + 0) * $indent, ' ');
                sformat(buf, k + 1 == node.count() ? cstr("</{}>") : cstr("</{}>\n"), k);
            }
            else {
                sformat(buf, "<{} type=\"{}\">", k, val.type());
                formatNode(buf, val, 0);
                sformat(buf, k + 1 == node.count() ? cstr("</{}>") : cstr("</{}>\n"), k); 
            }
        }
        break;
    }
    case Type::object: {
        auto k = 0u;
        for (auto itr = node.begin(); itr != node.end(); ++k, ++itr) {
            if (k != 0) {
                buf.appends((level + 0)*$indent, ' ');
            }

            const auto val = *itr;
            

            if (val.type() == Type::object || val.type() == Type::array) {
                sformat(buf, "<{} type=\"{}\">\n", itr.key(), val.type());
                formatNode(buf, val, level + 1);
                buf += "\n";
                buf.appends((level + 0)*$indent, ' ');
                sformat(buf, k + 1 == node.count() ? cstr("</{}>") : cstr("</{}>\n"), itr.key());
            }
            else {
                sformat(buf, "<{} type=\"{}\">", itr.key(), val.type());
                formatNode(buf, val, 0);
                sformat(buf, k + 1 == node.count() ? cstr("</{}>") : cstr("</{}>\n"), itr.key());
            }
        }
        break;
    }
    default:
        break;
    }
}

NMS_API void formatImpl(String& buf, const NodeEx& tree, StrView/*fmt*/) {
    const auto capicity = tree.count() * 8 + tree.strlen();
    buf.reserve(buf.count() + capicity);

    buf += "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";
    buf += "<data>\n";
    formatNode(buf, tree, 1);
    buf += "\n";
    buf += "</data>\n";
}

}


#pragma region unittest
namespace nms::serialization::xml
{

struct Object
{
    String a;
    u32x3  b;

    friend NodeEx& operator<<(NodeEx& obj, const Object& val) {
        obj["a"] << val.a;
        obj["b"] << val.b;
        return obj;
    }

    friend const NodeEx& operator>>(NodeEx& obj, Object& val) {
        obj["a"] >> val.a;
        obj["b"] >> val.b;
        return obj;
    }

    friend void formatImpl(String& buf, const Object& obj, StrView fmt) {
        sformat(buf, R"({{ "a": "{}", "b": [{}]}})", obj.a, obj.b);
    }
};


nms_test(serialization) {
    Object obj = { "hello",{ 1u, 2u, 3u } };

    Tree tree;
    tree << obj;
    auto out = format("{:xml}", tree);
    io::console::writeln("out = {}\n", out);
}


#pragma endregion

}