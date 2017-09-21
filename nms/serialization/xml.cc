#include <nms/test.h>
#include <nms/serialization/dom.h>

namespace nms::serialization
{

#pragma region format: xml

NMS_API void XDOM::_format_xml_begin(IString& buf) const {
    const auto& v = val();
    buf += "<xml? version=\"1.0\" encoding=\"utf-8\" ?>\n";
    sformat(buf, "<root type={}>", v.type());
}

NMS_API void XDOM::_format_xml_end(IString& buf) const {
    buf += "</root>";
}

NMS_API void XDOM::_format_xml(IString& buf, u32 level) const {
    static const auto $indent = 2;
    const auto& v = val();

    switch (v.type()) {
    case Type::null:    _format(buf, {}, "");           break;
    case Type::boolean: _format(buf, {}, v.bool_val_);  break;
    case Type::u8:      _format(buf, {}, v.u8_val_);    break;
    case Type::i8:      _format(buf, {}, v.i8_val_);    break;
    case Type::u16:     _format(buf, {}, v.u16_val_);   break;
    case Type::i16:     _format(buf, {}, v.i16_val_);   break;
    case Type::u32:     _format(buf, {}, v.u32_val_);   break;
    case Type::i32:     _format(buf, {}, v.i32_val_);   break;
    case Type::u64:     _format(buf, {}, v.u64_val_);   break;
    case Type::i64:     _format(buf, {}, v.i64_val_);   break;
    case Type::f32:     _format(buf, {}, v.f32_val_);   break;
    case Type::f64:     _format(buf, {}, v.f64_val_);   break;

    case Type::datetime:
        DateTime(v.i64_val_).format(buf, {});
        break;

    case Type::number: {
        _format(buf, {}, v.str());
        break;
    }

    case Type::key: case Type::string: {
        buf += "\"";
        _format(buf, {}, v.str());
        buf += "\"";
        break;
    }

    case Type::array: {
        buf += "\n";
        buf.appends(level*$indent, ' ');

        auto k = 0u;
        for (auto itr = begin(); itr != end(); ++k, ++itr) {
            const auto element = *itr;
            sformat(buf, "{ }<{} type=\"{}\">", (level + 1) * $indent, k, element.type());
            element._format_xml(buf, level + 1);
            sformat(buf, "</{}>", k);
            buf += "\n";
            buf.appends(level*$indent, ' ');
        }
        break;
    }
    case Type::object: {
        buf += "\n";
        buf.appends(level*$indent, ' ');

        auto k = 0u;
        for (auto itr = begin(); itr != end(); ++k, ++itr) {
            const auto element = *itr;
            const auto key     = itr.key();
            sformat(buf, "{ }<{} type=\"{}\">", $indent, key, element.type());
            element._format_xml(buf, level + 1);
            sformat(buf, "</{}>", key);
            buf += "\n";
            buf.appends(level*$indent, ' ');
        }
        break;
    }
    default:
        break;
    }
}
#pragma endregion

#pragma region unittest
struct TestObject
    : public IFormatable
    , public ISerializable
{
    NMS_PROPERTY_BEGIN;
    typedef String<32>    NMS_PROPERTY(a);
    typedef f32x4         NMS_PROPERTY(b);
    typedef DateTime      NMS_PROPERTY(c);
    NMS_PROPERTY_END;
};

nms_test(xml_serialize) {
    TestObject obj;
    obj.a = "hello";
    obj.b ={ 1.1f, +2.2f, -3.3f, 4.4e2f };
    obj.c = DateTime(2017, 9, 3, 8, 30, 12);

    Tree<64> node;
    node << obj;

    io::log::info("obj = {}", obj);
    io::log::info("xml = {:xml}", node);
}

#pragma endregion

}