#pragma once

#include <nms/core.h>
#include <nms/util/stacktrace.h>

namespace nms::serialization
{

namespace xml  {}
namespace json {}

struct Node;
struct NodeEx;

NMS_ENUM_EX(enum class Type : u16, Type,
    null,
    boolean,

    i8, u8,
    i16, u16,
    i32, u32,
    i64, u64,
    f32, f64,

    number,
    string,
    key,

    array,
    object,
    datetime,
    );

class EUnexpectType : public IException
{
public:
    EUnexpectType(Type expect, Type value)
        : expect_(expect)
        , value_(value)
    {}

    void format(String& buf) const override {
        sformat(buf, "unexpect type");
    }
protected:
    Type    expect_;
    Type    value_;
};

class EOutofRange
    : public IException
{
public:
    void format(String& buf) const override {
        sformat(buf, "out of range");
    }
};

class EUnexpectElementCount
    : public IException
{
public:
    EUnexpectElementCount(u32 expect, u32 value)
        : expect_(expect)
        , value_(value)
    {}

    void format(String& buf) const override {
        sformat(buf, "expect count = {}, but value = {}", expect_, value_);
    }
protected:
    u32     expect_;
    u32     value_;
};

class EKeyNotFound
    : public IException
{
public:
    EKeyNotFound(StrView key)
        : key_(key)
    {}

    void format(String& buf) const override {
        sformat(buf, "key = {}", key_);
    }

protected:
    StrView key_;
};


struct ISerializable
{
    friend struct NodeEx;

private:
    template<class T>
    static void _serialize(NodeEx& node, const T& obj) {
        serialize_begin<0>(&node, &obj, nullptr);
    }

    // serialize-begin
    template<u32 I, class T>
    static void serialize_begin(NodeEx* node, const T* obj, ...) {
        serialize_begin<I+1>(node, obj, nullptr);
        return;
    }

    // serialize-switch
    template<u32 I, class T>
    static auto serialize_begin(NodeEx* node, const T* obj, nullptr_t)-> decltype((*obj)[I32<I>{}], 0) {
        serialize_do<I>(node, obj);
        return 0;
    }

    // serialize-do
    template<u32 I, class T>
    static auto serialize_do(NodeEx* node, const T* obj)-> decltype((*obj)[I32<I>{}], 0) {
        auto src = (*obj)[I32<I>{}];
        auto dst = (*node)[src.name];
        dst << src.value;
        serialize_do<I + 1>(node, obj);
        return 0;
    }

    // serialize-end
    template<u32 I>
    static void serialize_do(...)
    {}

    template<class T>
    static void _deserialize(const NodeEx& node, T& obj) {
        deserialize_begin<0>(&node, &obj, nullptr);
    }

    // deserialize-begin
    template<u32 I, class T>
    static void deserialize_begin(const NodeEx* node, T* obj, ...) {
        deserialize_begin<I+1>(node, obj, nullptr);
    }

    // deserialize-switch
    template<u32 I, class T>
    static auto deserialize_begin(const NodeEx* node, T* obj, nullptr_t)-> decltype((*obj)[I32<I>{}], 0) {
        deserialize_do<I>(node, obj);
        return 0;
    }

    // deserialize-do
    template<u32 I, class T>
    static auto deserialize_do(const NodeEx* node, T* obj)-> decltype((*obj)[I32<I>{}], 0) {
        auto dst = (*obj)[I32<I>{}];
        auto src = (*node)[dst.name];
        src >> dst.value;
        deserialize_do<I + 1>(node, obj);
        return 0;
    }

    // deserialize-end
    template<u32 I>
    static void deserialize_do(...)
    {}
};

}
