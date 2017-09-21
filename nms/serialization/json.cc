#include <nms/test.h>
#include <nms/serialization/dom.h>

namespace nms::serialization
{

#pragma region format:json
NMS_API void XDOM::_format_json(IString& buf, u32 level) const {
    static const auto $indent = 4;

    auto& v = val();

    switch (v.type()) {
    case Type::null:    nms::_format(buf, {}, "");              break;
    case Type::boolean: nms::_format(buf, {}, v.bool_val_);     break;
    case Type::u8:      nms::_format(buf, {}, v.u8_val_);       break;
    case Type::i8:      nms::_format(buf, {}, v.i8_val_);       break;
    case Type::u16:     nms::_format(buf, {}, v.u16_val_);      break;
    case Type::i16:     nms::_format(buf, {}, v.i16_val_);      break;
    case Type::u32:     nms::_format(buf, {}, v.u32_val_);      break;
    case Type::i32:     nms::_format(buf, {}, v.i32_val_);      break;
    case Type::u64:     nms::_format(buf, {}, v.u64_val_);      break;
    case Type::i64:     nms::_format(buf, {}, v.i64_val_);      break;
    case Type::f32:     nms::_format(buf, {}, v.f32_val_);      break;
    case Type::f64:     nms::_format(buf, {}, v.f64_val_);      break;

    case Type::datetime: {
        buf += "\"";
        DateTime(v.i64_val_).format(buf, {});
        buf += "\"";
        break;
    }
    case Type::number: {
        nms::_format(buf, {}, v.str());
        break;
    }

    case Type::key: case Type::string: {
        buf += "\"";
        nms::_format(buf, {}, v.str());
        buf += "\"";
        break;
    }

    case Type::array: {
        buf += "[\n";

        for (auto itr = begin(); itr != end(); ) {
            buf.appends((level+1)*$indent, ' ');
            (*itr)._format_json(buf, level + 1);
            _format(buf, {}, (++itr == end()) ? StrView{ "\n" } : StrView{ ",\n" });
        }

        buf.appends(level * $indent, ' ');
        buf += "]";
        break;
    }
    case Type::object: {
        buf += "{\n";

        for (auto itr = begin(); itr != end();) {
            buf.appends((level + 1)*$indent, ' ');
            buf += "\"";
            buf += itr.key();
            buf += "\": ";
            (*itr)._format_json(buf, level + 1);
            _format(buf, {}, (++itr == end()) ? StrView{ "\n" } : StrView{ ",\n" });
        }
        buf.appends(level * $indent, ' ');
        buf += "}";
        break;
    }
    default:
        break;
    }
}
#pragma endregion

#pragma region parse:json
// parse
static bool expect(const StrView& expect, const StrView& text) {
    for (u32 i = 1; i < expect.count(); ++i) {
        if (expect[i] != text[i]) {
            return false;
        }
    }
    return true;
}

// test if blank
static bool isBlank(char c) {
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n') return true;
    return false;
}

static char peekChar(StrView& text) {
    u32 pos = 0;
    u32 len = u32(text.count());
    while (pos < len && isBlank(text[pos])) pos++;

    auto c = text[pos];
    text = text.slice(pos, u32(text.count() - 1));
    return c;
}

static char parseChar(StrView& text) {
    auto c = peekChar(text);
    text=text.slice(1u, u32(text.count()) - 1);
    return c;
}

static i32 parseAny(StrView& text, XDOM* nodes, i32 proot, i32 pleft);

static i32 parseNum(StrView& text, XDOM* nodes, i32 proot, i32 pleft) {
    // 0123456789,
    // ^         ^
    // s         p
    auto s = text.data();
    auto p = s;
    while (*p != ',' && *p != '}' && *p != ']' && *p != '\n') {
        ++p;
    }

    auto ret = nodes->add(proot, pleft, DOM(StrView{ s, u32(p - s) }, Type::number));
    text     = StrView{ p, u32(text.count() - 1) };
    return ret;
}

static i32 parseStr(StrView& text, XDOM* nodes, i32 proot, i32 pleft) {
    // "abcdefg"
    // ^ ......^
    // b       e
    auto ptr    = text.data();
    auto pos    = text.data() + 1;

    while (true) {
        auto c = *pos;

        if (c == '\\') {
            pos += 2;
        }
        else if (c == '"') {
            break;
        }
        else {
            pos += 1;
        }
    }

    const auto b = 0;
    const auto e = u32(pos - ptr);
    const auto ret = nodes->add(proot, pleft, DOM(StrView{ text.data() + b + 1, e - b - 1 }, Type::string));
    text = text.slice(e + 1, u32(text.count()) - 1);
    return ret;
}

static i32 parseKey(StrView& text, XDOM* nodes, i32 proot, i32 pleft) {
    // "abcdefg"
    // ^ ......^
    // b       e

    u32 b = 0;
    u32 e = 1;
    while (text[e] != '"' && text[e - 1] != '\\') ++e;
    auto ret = nodes->add(proot, pleft, DOM(StrView{ text.data() + b + 1,  e - b - 1 }, Type::key));
    text = text.slice(e + 1, u32(text.count()) - 1);
    return ret;
}

static i32 parseArray(StrView& text, XDOM* nodes, i32 proot, i32 pleft) {
    auto parr = nodes->add(proot, pleft, DOM(Type::array));

    text = text.slice(1u, u32(text.count()) - 1);
    if (peekChar(text) == ']') {
        parseChar(text);
        return parr;
    }

    auto prev_val = -1;
    while (true) {
        auto this_val = parseAny(text, nodes, parr, prev_val);
        prev_val = this_val;

        auto next_char = parseChar(text);
        if (next_char == ']') {
            return parr;
        }
        if (next_char == ',') {
            continue;
        }
        return -1;
    }
}

static i32 parseObject(StrView& text, XDOM* nodes, i32 proot, i32 pleft) {
    // add new node
    auto pobj = nodes->add(proot, pleft, DOM(Type::object));

    text = text.slice(1, -1);
    if (peekChar(text) == '}') {
        return pobj;
    }

    auto prev_key = -1;
    auto prev_val = -1;

    while (true) {
        // key
        const auto next_quota  = peekChar(text);
        if (next_quota != '"') {
            return -1;
        }
        const auto this_key  = parseKey(text, nodes, pobj, prev_key);

        const auto next_colon = parseChar(text);
        if (next_colon != ':') {
            io::log::error("nms.serialization.json.parse_object: expect ':', but '{:c}' ", next_colon);
            return -1;
        }
        const auto this_val  = parseAny(text, nodes, -1, prev_val);
        (void)this_val;

        prev_key = this_key;
        prev_val = this_val;

        const auto next_char = parseChar(text);
        if (next_char == '}') {
            break;
        }
        if (next_char == ',') {
            continue;
        }
        io::log::error("nms.serialization.json.parse_object: expect ',' or '}', but '{:c}' ", next_char);
        return -1;
    }
    return pobj;
}

static i32 parseAny(StrView& text, XDOM* nodes, i32 proot, i32 pleft) {
    auto result = -1;

    const auto text_len    = text.count();

    // peek next char
    const auto next_char = peekChar(text);

    // test char
    switch (next_char) {
    case 'n':
    {
        if (expect("null", text)) {
            text    = text.slice(4u, text_len - 1);
            result  = nodes->add(proot, pleft, DOM{ Type::null });
        }
        break;
    }
    case 't':
    {
        if (expect("true", text)) {
            text    = text.slice(4u, text_len - 1);
            result  = nodes->add(proot, pleft, DOM{ true });
        }
        break;
    }
    case 'f':
    {
        if (expect("false", text)) {
            text    = text.slice(5u, text_len - 1);
            result  = nodes->add(proot, pleft, DOM{ false });
        }
        break;
    }

    case '[':
    {
        result = parseArray(text, nodes, proot, pleft);
        break;
    }

    case '{':
    {
        result = parseObject(text, nodes, proot, pleft);
        break;
    }

    case '"':
    {
        result = parseStr(text, nodes, proot, pleft);
        break;
    }

    case '+':
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    {
        result = parseNum(text, nodes, proot, pleft);
        break;
    }

    default:
        break;
    }

    return result;
}

// wraper
NMS_API void XDOM::_parse_json(const StrView& text) {
    auto str = text;
    parseAny(str, this, -1, -1);
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

nms_test(json_serialize) {
    TestObject obj;
    obj.a = "hello";
    obj.b ={ 1.1f, +2.2f, -3.3f, 4.4e2f };
    obj.c = DateTime(2017, 9, 3, 8, 30, 12);

    Tree<64> node;
    node << obj;

    io::log::info("obj  = {}", obj);
    io::log::info("json = {:json}", node);
}

nms_test(json_deserialize) {
    const char text[] = R"(
{
    "a": "hello",
    "b": [ 1.1, +2.2, -3.3, 4.4e2],
    "c": "2017-09-03T08:30:12"
}
)";
    // json_str -> json_tree
    auto dom = Tree<32>(text, $json);

    TestObject val;
    dom >> val;

    io::log::debug("obj = {}", val);
}

#pragma endregion

}
