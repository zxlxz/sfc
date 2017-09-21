#pragma once

#include <nms/core.h>

namespace nms::serialization
{

enum DOMType
{
    $json,
    $xml
};

struct DOM;
struct XDOM;

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
    datetime
    );

class EUnexpectType : public IException
{
public:
    EUnexpectType(Type expect, Type value)
        : expect_(expect)
        , value_(value)
    {}

    void format(IString& buf) const override {
        sformat(buf, "expect=`{}`, value=`{}`", expect_, value_);
    }
protected:
    Type    expect_;
    Type    value_;
};

class EUnexpectElementCount
    : public IException
{
public:
    EUnexpectElementCount(u32 expect, u32 value)
        : expect_(expect)
        , value_(value)
    {}

    void format(IString& buf) const override {
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

    void format(IString& buf) const override {
        sformat(buf, "key = {}", key_);
    }

protected:
    StrView key_;
};

struct ISerializable
{
    friend struct XDOM;

protected:
    template<class T>
    static void _serialize(XDOM& node, const T& obj) {
#ifndef NMS_CC_INTELLISENSE
#define call_serialize_impl(n, ...)    _serialize_impl(Ti32<n>{}, &obj, &node);
        NMSCPP_LOOP(99, call_serialize_impl);
#undef call_serialize_impl
#endif
    }

    template<class T>
    static void _deserialize(const XDOM& node, T& obj) {
#ifndef NMS_CC_INTELLISENSE
#define call_deserialize_impl(n, ...)    _deserialize_impl(Ti32<n>{}, &obj, &node);
        NMSCPP_LOOP(99, call_deserialize_impl);
#undef call_deserialize_impl
#endif
    }

private:
    // serialize-impl
    template<class T, i32 I>
    static auto _serialize_impl(Ti32<I> idx, const T* pobj, XDOM* pnod)->$when<(I < T::_$property_cnt)> {
        auto obj_item = (*pobj)[idx];
        auto nod_item = (*pnod)[obj_item.name];
        nod_item << (*obj_item.pval);
        return;
    }

    // serialize-impl
    template<class T, i32 I >
    static auto _serialize_impl(Ti32<I>, const T* pobj, ...) -> $when<(I >= T::_$property_cnt)> {
        (void)pobj;
        return;
    }

    // deserialize-impl
    template<class T, i32 I>
    static auto _deserialize_impl(Ti32<I> idx, T* pobj, const XDOM* pnod)->$when<(I < T::_$property_cnt)> {
        auto obj_item = (*pobj)[idx];
        auto nod_item = (*pnod)[obj_item.name];
        nod_item >> (*obj_item.pval);
        return;
    }

    // deserialize-impl
    template<class T, i32 I >
    static auto _deserialize_impl(Ti32<I>, T* pobj, ...) -> $when<(I >= T::_$property_cnt)> {
        (void)pobj;
        return;
    }
};

}
