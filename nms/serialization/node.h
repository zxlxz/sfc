#pragma once

#include <nms/serialization/base.h>

namespace  nms::serialization
{

namespace json
{
static void formatNode(String& buf, const NodeEx& node, i32 level);
}

namespace xml
{
static void formatNode(String& buf, const NodeEx& node, i32 level);
}

struct Node
{
    friend struct NodeEx;
    friend struct NodeIterator;
    friend class  Tree;

    friend void json::formatNode(String& buf, const NodeEx& node, i32 level);
    friend void  xml::formatNode(String& buf, const NodeEx& node, i32 level);

public:
    Node() = default;

    explicit Node(bool    val) : type_(Type::boolean), bool_val_(val){}

    explicit Node(u8      val) : type_(Type::u8),   u8_val_(val) {}
    explicit Node(i8      val) : type_(Type::i8),   i8_val_(val) {}

    explicit Node(u16     val) : type_(Type::u16),  u16_val_(val) {}
    explicit Node(i16     val) : type_(Type::i16),  i16_val_(val) {}

    explicit Node(u32     val) : type_(Type::u32),  u32_val_(val) {}
    explicit Node(i32     val) : type_(Type::i32),  i32_val_(val) {}

    explicit Node(u64     val) : type_(Type::u64),  u64_val_(val) {}
    explicit Node(i64     val) : type_(Type::i64),  i64_val_(val) {}
    explicit Node(f32     val) : type_(Type::f32),  f32_val_(val) {}
    explicit Node(f64     val) : type_(Type::f64),  f64_val_(val) {}

    explicit Node(StrView val, Type type=Type::string)
        : type_(type), size_(val.count()), str_val_(val.data())
    {}

    explicit Node(Type type, u32 size = 0)
        : type_(type), size_(size), str_val_(nullptr)
    {}

    Type type() const {
        return type_;
    }

    u32 count() const {
        return size_;
    }

    u32 size() const {
        return size_;
    }

    u32 next() const {
        return next_;
    }

    StrView str() const {
        if ((type_ != Type::string) && (type_ != Type::key) && (type_ != Type::number) ) {
            throw EUnexpectType(Type::string, type_);
        }
        return { str_val_, {size_} };
    }

protected:
    using str_t = const char*;

    Type    type_ = Type::null;  // 2 byte
    u16     size_ = 0;           // 2 byte
    u32     next_ = 0;           // 4 byte

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

        str_t   num_val_;
        str_t   str_val_;
        str_t   key_val_;

        Node*   arr_val_;
        Node*   obj_val_ = nullptr;
    };
};


struct NodeEx
{

#pragma region constructort
public:
    constexpr NodeEx(List<Node>& lst, i32 idx) noexcept
        : lst_(lst)
        , idx_(idx)
    {}
#pragma endregion

#pragma region iterator
public:
    struct Iterator
    {
        friend struct NodeEx;
    public:
        Iterator(List<Node>& lst, i32 idx)
            : lst_(lst), idx_(idx)
        {}

        Iterator& operator++() {
            auto& v = lst_[idx_];
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
            auto v = lst_[idx_];
            if (v.next() != 0) {
                idx_ += v.next();
            }
            else {
                idx_ = 0;
            }
            return x;
        }

        StrView key() const {
            auto& x = lst_[idx_ - 1];
            if (x.type_ != Type::key) {
                throw EUnexpectType{ Type::key, x.type_ };
            }
            return { x.key_val_, {x.size_} };
        }

        NodeEx operator*() const {
            return { lst_, idx_ };
        }

        friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
            return lhs.idx_ == rhs.idx_;
        }

        friend bool operator!=(const Iterator& lhs, const Iterator& rhs) {
            return lhs.idx_ != rhs.idx_;
        }

    protected:
        List<Node>&  lst_;
        i32          idx_;
    };

    /* iterator: begin */
    Iterator begin() const {
        auto type = this->type();
        if (size() == 0) {
            return { lst_, 0 };
        }
        if (type == Type::array) {
            return { lst_, idx_ + 1 };
        }
        else if (type == Type::object) {
            return { lst_, idx_ + 2 };
        }
        else {
            throw EUnexpectType(Type::array, type);
        }
    }

    /* iterator: end */
    Iterator end() const {
        return { lst_, 0 };
    }

#pragma endregion

#pragma region property

    Type type() const {
        return lst_[idx_].type_;
    }

    u32 size() const {
        return lst_[idx_].size_;
    }

    u32 count() const {
        return lst_[idx_].size_;
    }


    /* get key */
    StrView key() const {
        auto k = lst_[idx_ - 1];
        if (k.type_ != Type::key) {
            throw EUnexpectType{ Type::key, k.type_ };
        }
        StrView val = { k.str_val_, {k.size_} };
        return val;
    }

    /* get val */
    Node& val() {
        return lst_[idx_];
    }

    /* get val */
    const Node& val() const {
        return lst_[idx_];
    }

    template<class T>
    operator T() const {
        T val;
        *this >> val;
        return val;
    }

    template<class T>
    auto& operator=(const T& t) {
        *this << t;
        return *this;
    }
#pragma endregion

#pragma region core types
    auto& operator<<(bool     x) { set_val(Node(x));          return *this; }
    auto& operator<<(i8       x) { set_val(Node(x));          return *this; }
    auto& operator<<(u8       x) { set_val(Node(x));          return *this; }
    auto& operator<<(i16      x) { set_val(Node(x));          return *this; }
    auto& operator<<(u16      x) { set_val(Node(x));          return *this; }
    auto& operator<<(i32      x) { set_val(Node(x));          return *this; }
    auto& operator<<(u32      x) { set_val(Node(x));          return *this; }
    auto& operator<<(i64      x) { set_val(Node(x));          return *this; }
    auto& operator<<(u64      x) { set_val(Node(x));          return *this; }
    auto& operator<<(f32      x) { set_val(Node(x));          return *this; }
    auto& operator<<(f64      x) { set_val(Node(x));          return *this; }
    auto& operator<<(StrView  x) { set_val(Node(x));          return *this; }

    auto& operator>>(bool&    x) const { get_val(x, Type::boolean); return *this; }
    auto& operator>>(i8&      x) const { get_val(x, Type::i8);      return *this; }
    auto& operator>>(u8&      x) const { get_val(x, Type::u8);      return *this; }
    auto& operator>>(i16&     x) const { get_val(x, Type::i16);     return *this; }
    auto& operator>>(u16&     x) const { get_val(x, Type::u16);     return *this; }
    auto& operator>>(i32&     x) const { get_val(x, Type::i32);     return *this; }
    auto& operator>>(u32&     x) const { get_val(x, Type::u32);     return *this; }
    auto& operator>>(i64&     x) const { get_val(x, Type::i64);     return *this; }
    auto& operator>>(u64&     x) const { get_val(x, Type::u64);     return *this; }
    auto& operator>>(f32&     x) const { get_val(x, Type::f32);     return *this; }
    auto& operator>>(f64&     x) const { get_val(x, Type::f64);     return *this; }
    auto& operator>>(StrView& x) const { get_val(x, Type::string);  return *this; }

#pragma region string
    auto& operator<<(const String& x)  {
        *this << StrView(x);         
        return *this;
    }

    auto& operator>>(String&  x) const {
        StrView y;
        *this >> y;
        x = y;
        return *this;
    }
#pragma endregion

#pragma region array
    /* array: index */
    NMS_API NodeEx operator[](u32 k);

    /* array: index */
    NMS_API NodeEx operator[](u32 k)     const;

#pragma endregion

#pragma region object
    /* object: index */
    NMS_API NodeEx operator[](StrView k);

    /* object: index */
    NMS_API NodeEx operator[](StrView k) const;

    /* object: find */
    NMS_API Iterator find(StrView) const;

    /* object: index */
    template<u32 N>
    NodeEx operator[](const char(&s)[N]) {
        return (*this)[cstr(s)];
    }

    /* object: index */
    template<u32 N>
    NodeEx operator[](const char(&s)[N]) const {
        return (*this)[cstr(s)];
    }
#pragma endregion

#pragma region vec,list
    template<class T, u32 N>
    const NodeEx& operator>>(Vec<T,N>& vec) const {
        if (type() != Type::array) {
            throw EUnexpectType(Type::array, type());
        }
        const auto n = count();
        if (n != N) {
            throw EUnexpectElementCount{ N, n };
        }
        auto i = 0u;
        for (auto e : *this) {
            T val;
            e >> val;
            vec[i++] = move(val);
        }

        return *this;
    }

    template<class T, u32 N>
    NodeEx& operator<<(const Vec<T, N>& x) {
        set_val(Node(Type::array));

        auto root = idx_;
        auto prev = 0;
        for (auto i = 0u; i < N; ++i) {
            prev = add(root, prev, Node(Type::null));
            NodeEx{ lst_, prev } << x[i];
        }
        return *this;
    }

    /* node -> list */
    template<class T, u32 S>
    const NodeEx& operator>>(List<T,S>& x) const {
        if (type() != Type::array) {
            throw EUnexpectType{ Type::array, type() };
        }
        const auto n = count();
        x.reserve(n);
        for (auto e : *this) {
            T val;
            e >> val;
            x.append(move(val));
        }

        return *this;
    }

    /* node <- list */
    template<class T, u32 S>
    NodeEx& operator<<(const List<T, S>& x) {
        set_val(Node(Type::array));

        auto root = idx_;
        auto prev = 0;

        const auto n = x.count();
        for (auto i = 0u; i < n; ++i) {
            prev = add(root, prev, Node(Type::null));
            NodeEx{ lst_, prev } << x[i];
        }
        return *this;
    }
#pragma endregion

#pragma region enum
    /* node -> enum */
    template<class T>
    auto operator>>(T& x) const -> $when<$is_enum<T>, const NodeEx&> {
        StrView str = val().str();
        x = parse<T>(str);
        return *this;
    }

    /* node <- enum */
    template<class T>
    auto operator<<(const T& x) -> $when<$is_enum<T>, NodeEx&> {
        StrView str = str_cast(x);
        *this << str;
        return *this;
    }
#pragma endregion

#pragma region struct
    /* node -> struct */
    template<class T>
    auto operator>>(T& x) const -> $when<$is_base_of<ISerializable,T>, const NodeEx&> {
        ISerializable::_deserialize(*this, x);
        return *this;
    }

    /* node <- struct */
    template<class T>
    auto operator<<(const T& x) -> $when<$is_base_of<ISerializable,T>, NodeEx&> {
        ISerializable::_serialize(*this, x);
        return *this;
    }
#pragma endregion

#pragma region utils
    u32 strlen() const {
        if (lst_.count() == 0) {
            return 0;
        }
        return lst_[0].size();
    }

    NMS_API void format(String& buf, StrView fmt) const;

    NMS_API i32 add(i32 root, i32 prev, const Node& val);

    NMS_API i32 add(i32 root, i32 prev, StrView key, const Node& val);
#pragma endregion

#pragma region protected
protected:
    List<Node>& lst_;
    i32         idx_;

    void get_val(StrView& x, Type t) const {
        auto& v = val();
        x = v.str();
    }

    void get_val(bool & x, Type t) const {
        auto& v = val();
        if (v.type() == t) {
            x = v.bool_val_;
            return;
        }

        throw EUnexpectType{ Type::boolean, v.type() };
    }

    template<class T>
    void get_val(T& x, Type t) const {
        auto& v = val();
        auto  p = const_cast<T*>(reinterpret_cast<const T*>(&v.obj_val_));

        if (v.type() == t) {
            x = *p;
            return;
        }

        if (v.type() == Type::number) {
            const_cast<T&>(*p)          = parse<T>( v.str());
            const_cast<Type&>(v.type_)  = t;
            return;
        }
        throw EUnexpectType{ Type::number, v.type() };
    }

    void set_val(const Node& x) {
        auto& v = val();

        if (v.type() == Type::null || v.type() == x.type() ) {
            v = x;
        }
        else {
            throw EUnexpectType{ v.type(), x.type() };
        }
    }
#pragma endregion

};

class Tree
    : public INocopyable
    , public NodeEx
{
    using base = NodeEx;

public:
    Tree()
        : base(nodes_, 0)
    {}

    ~Tree()
    {}

    Tree(Tree&& rhs) noexcept
        : base(nodes_, rhs.idx_)
        , nodes_{ move(rhs.nodes_) }
    {}

    Tree& operator=(Tree&& rhs) noexcept {
        nms::swap(base::idx_, rhs.idx_);
        nms::swap(nodes_,     rhs.nodes_);
        return *this;
    }

protected:
    List<Node>  nodes_;
};

}

