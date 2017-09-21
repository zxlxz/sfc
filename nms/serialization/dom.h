#pragma once

#include <nms/serialization/base.h>

namespace  nms::serialization
{

struct DOM
{
    friend struct XDOM;
    friend struct NodeIterator;

    using Tsize = u16;
    using Tnext = i32;
public:
    DOM() = default;

    explicit DOM(bool      val) : type_(Type::boolean),    bool_val_(val){}

    explicit DOM(u8        val) : type_(Type::u8),         u8_val_(val) {}
    explicit DOM(i8        val) : type_(Type::i8),         i8_val_(val) {}

    explicit DOM(u16       val) : type_(Type::u16),        u16_val_(val) {}
    explicit DOM(i16       val) : type_(Type::i16),        i16_val_(val) {}

    explicit DOM(u32       val) : type_(Type::u32),        u32_val_(val) {}
    explicit DOM(i32       val) : type_(Type::i32),        i32_val_(val) {}

    explicit DOM(u64       val) : type_(Type::u64),        u64_val_(val) {}
    explicit DOM(i64       val) : type_(Type::i64),        i64_val_(val) {}
    explicit DOM(f32       val) : type_(Type::f32),        f32_val_(val) {}
    explicit DOM(f64       val) : type_(Type::f64),        f64_val_(val) {}
    explicit DOM(DateTime  val) : type_(Type::datetime),   time_val_(val.stamp()) {}

    explicit DOM(StrView   val, Type type=Type::string)
        : type_(type), size_(Tsize(val.count())), str_val_(val.data())
    {}

    explicit DOM(Type type, Tsize size = 0)
        : type_(type), size_(size), str_val_(nullptr)
    {}

    Type type() const {
        return type_;
    }

    Tsize count() const {
        return size_;
    }

    Tsize size() const {
        return size_;
    }

    Tnext next() const {
        return next_;
    }

    StrView str() const {
        if (type_ == Type::null) {
            return {};
        }

        if ((type_ != Type::string) && (type_ != Type::key) && (type_ != Type::number) ) {
            NMS_THROW(EUnexpectType(Type::string, type_));
        }
        return { str_val_, size_ };
    }

protected:
    using str_t = const char*;

    Type    type_ = Type::null;  // 2 byte
    Tsize   size_ = 0;           // 2 byte
    Tnext   next_ = 0;           // 4 byte

    union                        // 8byte
    {
        bool    bool_val_;

        i8      i8_val_;
        i16     i16_val_;
        i32     i32_val_;
        i64     i64_val_;

        u8      u8_val_;
        u16     u16_val_;
        u32     u32_val_;
        u64     u64_val_;

        f32     f32_val_;
        f64     f64_val_;

        i64     time_val_;

        str_t   num_val_;
        str_t   str_val_;
        str_t   key_val_;

        DOM*   arr_val_;
        DOM*   obj_val_ = nullptr;
    };
};

struct XDOM
{

#pragma region constructort
public:
    constexpr XDOM(IList<DOM>* lst, i32 idx) noexcept
        : pnodes_(lst)
        , index_(idx)
    {}
#pragma endregion

#pragma region iterator
public:
    struct Iterator
    {
        friend struct XDOM;
    public:
        Iterator(IList<DOM>* lst, i32 idx)
            : lst_(lst), idx_(idx)
        {}

        Iterator& operator++() {
            auto& v = (*lst_)[idx_];
            if (v.next() != 0) {
                idx_ += v.next();
            }
            else {
                idx_ = 0;
            }
            return *this;
        }

        Iterator operator++(int) {
            auto x = *this;
            auto v = (*lst_)[idx_];
            if (v.next() != 0) {
                idx_ += v.next();
            }
            else {
                idx_ = 0;
            }
            return x;
        }

        StrView key() const {
            auto& x = (*lst_)[idx_ - 1];
            if (x.type_ != Type::key) {
                NMS_THROW(EUnexpectType{ Type::key, x.type_ });
            }
            return { x.key_val_, x.size_ };
        }

        XDOM operator*() const {
            return { lst_, idx_ };
        }

        XDOM operator->() const {
            return { lst_, idx_ };
        }

        friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
            return lhs.idx_ == rhs.idx_;
        }

        friend bool operator!=(const Iterator& lhs, const Iterator& rhs) {
            return lhs.idx_ != rhs.idx_;
        }

    protected:
        IList<DOM>*  lst_;
        i32          idx_;
    };

    /* iterator: begin */
    Iterator begin() const {
        auto type = this->type();
        if (size() == 0) {
            return { pnodes_, 0 };
        }
        if (type == Type::array) {
            return { pnodes_, index_ + 1 };
        }
        if (type == Type::object) {
            return { pnodes_, index_ + 2 };
        }

        NMS_THROW(EUnexpectType(Type::array, type));
    }

    /* iterator: end */
    Iterator end() const {
        return { pnodes_, 0 };
    }

#pragma endregion

#pragma region property

    Type type() const {
        return val().type_;
    }

    u32 size() const {
        return val().size_;
    }

    u32 count() const {
        return val().size_;
    }

    /* get key */
    StrView key() const {
        auto k = (*pnodes_)[index_ - 1];
        if (k.type_ != Type::key) {
            NMS_THROW(EUnexpectType{ Type::key, k.type_ });
        }
        StrView val = { k.str_val_, k.size_ };
        return val;
    }

    /* get val */
    __forceinline DOM& val() {
        return (*pnodes_)[index_];
    }

    /* get val */
    __forceinline const DOM& val() const {
        return (*pnodes_)[index_];
    }


#pragma endregion

#pragma region array
    /* array: index */
    NMS_API XDOM operator[](u32 k);

    /* array: index */
    NMS_API XDOM operator[](u32 k)     const;

#pragma endregion

#pragma region object
    /* object: index */
    NMS_API XDOM operator[](StrView k);

    /* object: index */
    NMS_API XDOM operator[](StrView k) const;

    /* object: find */
    NMS_API Iterator find(StrView) const;

    /* object: index */
    template<u32 N>
    XDOM operator[](const char(&s)[N]) {
        return (*this)[StrView(s)];
    }

    /* object: index */
    template<u32 N>
    XDOM operator[](const char(&s)[N]) const {
        return (*this)[StrView(s)];
    }
#pragma endregion

#pragma region get/set
    template<class T>
    const XDOM& operator>>(T& x) {
#ifndef NMS_CC_INTELLISENSE
        get(x);
#endif
        return *this;
    }

    template<class T>
    XDOM& operator<<(const T& x) {
#ifndef NMS_CC_INTELLISENSE
        set(x);
#endif
        return *this;
    }

    template<class T>
    operator T() const {
        T val;
        get(val);
        return val;
    }

    template<class T>
    auto& operator=(const T& t) {
        set(t);
        return *this;
    }

    void get(i8&      x) const { get_num(x, Type::i8);  }
    void get(u8&      x) const { get_num(x, Type::u8);  }
    void get(i16&     x) const { get_num(x, Type::i16); }
    void get(u16&     x) const { get_num(x, Type::u16); }
    void get(i32&     x) const { get_num(x, Type::i32); }
    void get(u32&     x) const { get_num(x, Type::u32); }
    void get(i64&     x) const { get_num(x, Type::i64); }
    void get(u64&     x) const { get_num(x, Type::u64); }
    void get(f32&     x) const { get_num(x, Type::f32); }
    void get(f64&     x) const { get_num(x, Type::f64); }

    void get(bool&    x) const { get_bool(x);   }
    void get(StrView& x) const { get_str(x);    }
    void get(DateTime&x) const { get_time(x);   }

    void set(bool     x) { set_node(DOM(x)); }
    void set(i8       x) { set_node(DOM(x)); }
    void set(u8       x) { set_node(DOM(x)); }
    void set(i16      x) { set_node(DOM(x)); }
    void set(u16      x) { set_node(DOM(x)); }
    void set(i32      x) { set_node(DOM(x)); }
    void set(u32      x) { set_node(DOM(x)); }
    void set(i64      x) { set_node(DOM(x)); }
    void set(u64      x) { set_node(DOM(x)); }
    void set(f32      x) { set_node(DOM(x)); }
    void set(f64      x) { set_node(DOM(x)); }
    void set(StrView  x) { set_node(DOM(x)); }
    void set(DateTime x) { set_node(DOM(x)); }

#pragma region string
    template<u32 Icapicity>
    void get(List<char, Icapicity>& x) const {
        StrView s;
        get(s);
        x = s;
    }

    template<u32 Icapicity>
    void set(const List<char, Icapicity>& x) {
        set(StrView(x));
    }
#pragma endregion

#pragma region vec
    template<class T, u32 N>
    void get(Vec<T, N>& vec) const {
        if (type() != Type::array) {
            NMS_THROW(EUnexpectType(Type::array, type()));
        }
        const auto n = count();
        if (n != N) {
            NMS_THROW(EUnexpectElementCount{ N, n });
        }
        auto i = 0u;
        for (auto e : *this) {
            T val;
            e.get(val);
            vec[i++] = move(val);
        }
    }

    template<class T, u32 N>
    void set(const Vec<T, N>& x) {
        set_node(DOM(Type::array));

        auto root = index_;
        auto prev = 0;
        for (auto i = 0u; i < N; ++i) {
            prev = add(root, prev, DOM(Type::null));
            XDOM{ pnodes_, prev }.set(x[i]);
        }
    }

#pragma endregion

#pragma region list
    template<class T, u32 S>
    void get(List<T, S>& x) const {
        if (type() != Type::array) {
            NMS_THROW(EUnexpectType{ Type::array, type() });
        }
        const auto n = count();
        x.reserve(n);
        for (auto e : *this) {
            T val;
            e.get(val);
            x.append(move(val));
        }
    }

    /* node <- list */
    template<class T, u32 S>
    void set(const List<T, S>& x) {
        set_node(DOM(Type::array));

        auto root = index_;
        auto prev = 0;

        const auto n = x.count();
        for (auto i = 0u; i < n; ++i) {
            prev = add(root, prev, DOM(Type::null));
            XDOM{ pnodes_, prev }.set(x[i]);
        }
    }
#pragma endregion

#pragma region enum
    /* node -> enum */
    template<class Tenum>
    void get(Tenum& x, $when_is<$enum, Tenum>* = nullptr) const {
        auto& v = const_cast<DOM&>(val());

        if (v.type() == Type::number || v.type() == Type::i32) {
            i32 uval = 0;
            get(uval);
            x = Tenum(uval);
        }
        else if (v.type() == Type::string) {
            StrView s = v.str();
            Tenum   e = Enum<Tenum>::parse(s);
            x = e;
        }
    }

    /* node <- enum */
    template<class Tenum>
    void set(const Tenum& x, $when_is<$enum, Tenum>* =nullptr) {
        const StrView s = mkEnum(x).name();
        set(s);
    }
#pragma endregion

#pragma region serializable
    /* node -> struct */
    template<class Tserializable>
    void get(Tserializable& x, $when_is<ISerializable, Tserializable>* = nullptr) const {
        ISerializable::_deserialize(*this, x);
    }

    /* node <- struct */
    template<class Tserializable>
    void set(const Tserializable& x, $when_is<ISerializable, Tserializable>* = nullptr) {
        ISerializable::_serialize(*this, x);
    }
#pragma endregion

#pragma endregion

#pragma region utils
    u32 strlen() const {
        if (pnodes_->count() == 0) {
            return 0;
        }
        return (*pnodes_)[0].size();
    }

    NMS_API i32 add(i32 root, i32 prev, const DOM& val);

    NMS_API i32 add(i32 root, i32 prev, const StrView& key, const DOM& val);
#pragma endregion

#pragma region format
    void format(IString& buf, const StrView& fmt) const {
        if (fmt == "xml") {
            format(buf, $xml);
        }
        else {
            format(buf, $json);
        }
    }

    void format(IString& buf, DOMType type) const {
        switch (type) {
        case $json:
            _format_json(buf);
            break;
        case $xml:
            _format_xml_begin(buf);
            _format_xml(buf);
            _format_xml_end(buf);
            break;
        }
    }
#pragma endregion

#pragma region protected
protected:
    IList<DOM>* pnodes_;
    i32         index_;

    void set_node(const DOM& x) {
        auto& v = val();

        if (v.type() == Type::null || v.type() == x.type() ) {
            v = x;
        }
        else {
            NMS_THROW(EUnexpectType{ v.type(), x.type() });
        }
    }

    template<class T>
    bool get_num(T& x, Type t) const {
        auto v = val();

        auto p = reinterpret_cast<const T*>(&v.obj_val_);

        // type match
        if (v.type() == t) {
            x = *p;
            return true;
        }

        // type is number, then convert
        if (v.type() == Type::number) {
            const auto s = v.str();
            nms::parse(s, x);

            mutable_cast(*p)      = x;
            mutable_cast(v.type_) = t;
            return true;
        }

        // type is null
        if (v.type() == Type::null) {
            return false;
        }

        NMS_THROW(EUnexpectType{ Type::number, v.type() });
    }

    bool get_bool(bool& x) const {
        auto v = val();

        // type: match
        if (v.type() == Type::boolean) {
            x = v.bool_val_;
            return true;
        }
        // type: null
        if (v.type() == Type::null) {
            return false;
        }
        NMS_THROW(EUnexpectType{ Type::boolean, v.type() });
    }

    bool get_str(StrView& x) const {
        auto v = val();
        // type: match
        if (v.type() == Type::key || v.type() == Type::string) {
            x = v.str();
            return true;
        }
        // type: null
        if (v.type() == Type::null) {
            return false;
        }
        NMS_THROW(EUnexpectType{ Type::string, v.type() });
    }

    bool get_time(DateTime& x) const {
        auto v = val();
        // type: match
        if (v.type() == Type::datetime) {
            x = DateTime(v.time_val_);
            return true;
        }
        // type: string
        if (v.type() == Type::string) {
            auto s = v.str();
            auto t = DateTime::parse(s);
            x = t;

            mutable_cast(v.i64_val_) = t.stamp();
            mutable_cast(v.type_)    = Type::datetime;
            return true;
        }
        // type: null
        if (v.type() == Type::null) {
            return false;
        }
        NMS_THROW(EUnexpectType{ Type::string, v.type() });
    }

#pragma endregion


#pragma region parse/format
protected:
    void _parse(const StrView& str, DOMType type) {
        switch (type) {
        case $json:
            _parse_json(str);
            break;
        case $xml:
        default:
            break;
        }
    }
private:
    NMS_API void _format_json(IString& buf, u32 level = 0) const;

    NMS_API void _format_xml_begin(IString& buf) const;
    NMS_API void _format_xml_end  (IString& buf) const;
    NMS_API void _format_xml      (IString& buf, u32 level = 0) const;

    NMS_API void _parse_json(const StrView& str);
#pragma endregion

};

template<u32 Icapicity=0>
class Tree
    : public XDOM
{
public:
    constexpr static const auto $capicity = Icapicity;

    using base  = XDOM;
    using Tdata = List<DOM, $capicity>;

    Tree()
        : base(&nodes_, 0)
        , nodes_{}
    {}

    ~Tree()
    {}

    Tree(Tree&& rhs) noexcept
        : base(&nodes_, rhs.index_)
        , nodes_{ static_cast<Tdata&&>(rhs.nodes_) }
    {}

    Tree(const StrView& str, DOMType type)
        : Tree()
    {
        base::_parse(str, type);
    }

    Tree(const Tree&)               = delete;
    Tree& operator=(Tree&&)         = delete;
    Tree& operator=(const Tree&)    = delete;

    void reserve(u32 cnt) {
        nodes_.reserve(cnt);
    }
protected:
    Tdata nodes_;
};

}
