#include <nms/core.h>
#include <nms/serialization/node.h>
#include <nms/serialization/json.h>
#include <nms/serialization/xml.h>
#include <nms/io/log.h>

namespace nms::serialization
{

NMS_API NodeEx NodeEx::operator[](u32 k) const {
    if ( type() != Type::array) {
        throw_exception(EUnexpectType{ Type::array, type() });
    }

    const auto n = count();
    if (k >= n) {
        throw_exception(EUnexpectElementCount{ k + 1, n });
    }

    // find index
    auto itr = begin();
    for (auto i = 0u; i < k; ++i) {
        ++itr;
    }
    return *itr;
}

NMS_API NodeEx NodeEx::operator[](u32 k) {
    if (lst_.count() == 0) {
        lst_.append(Node{ Type::null, 0 });
        lst_.append(Node{ Type::array });
        this->idx_ = 1;
    }
    else if (type() != Type::array) {
        throw_exception(EUnexpectType{ Type::array, type() });
    }

    // find index
    auto itr = begin();
    for (auto i = 0u; i < k; ++i) {
        // out of range, insert new val
        if (i >= count()) {
            auto pnode = add(idx_, itr.idx_, Node());
            (void)pnode;
        }
        ++itr;
    }
    return *itr;
}

NMS_API NodeEx::Iterator NodeEx::find(StrView expect) const {
    if (type() != Type::object) {
        throw_exception(EUnexpectType{ Type::object, type() });
    }

    auto n      = count();
    auto itr    = begin();
    for (u32 i = 0; i < n; ++i, ++itr) {
        auto key = itr.key();
        if (key == expect) {
            return { lst_, itr.idx_ };
        }
    }
    return { lst_, 0 };
}

NMS_API NodeEx NodeEx::operator[](StrView key) const {
    auto itr = find(key);
    if (itr != end()) {
        return *itr;
    }

    throw_exception(EKeyNotFound{ key });
    return *itr;
}


NMS_API NodeEx NodeEx::operator[](StrView key) {
    if (lst_.count() == 0) {
        lst_.append(Node{ Type::null, 0 });
        lst_.append(Node{ Type::object });
        this->idx_ = 1;
        auto idx = add(1, 0, key, Node{ Type::null });
        return { lst_, idx };
    }

    auto& v = val();
    if (type() == Type::null) {
        v.type_ = Type::object;
    }
    else if (type() != Type::object) {
        throw_exception(EUnexpectType{ Type::object, type() });
    }

    auto n      = v.count();
    auto itr    = begin();
    auto last   = itr.idx_;

    for (u32 i = 0; i < n; ++i, ++itr) {
        last = itr.idx_;
        if (itr.key() == key) {
            return { lst_, itr.idx_ };
        }
    }

    // not find, insert new key/val
    auto pval = add(idx_, last, key, Node(Type::null));

    return NodeEx{ lst_, pval };
}

NMS_API i32 NodeEx::add(i32 root, i32 prev, const Node& val) {
    auto xpos = lst_.count();
    if (xpos == 0) {
        lst_.append(Node{ Type::null, 0 });
        this->idx_ = 1;
        ++xpos;
    }
    lst_.append(val);

    if (root > 0) {
        lst_[root].size_ += 1;
    }
    if (prev > 0) {
        lst_[prev].next_ = u32(xpos - prev);
    }
    if (val.type() == Type::string) {
        lst_[0].size_ += u32(val.size());
    }
    return xpos;
}

NMS_API i32 NodeEx::add(i32 root, i32 prev, StrView key, const Node& val) {
    auto xpos = lst_.count() + 1;
    if (xpos == 1) {
        lst_.append(Node{ Type::null, 0 });
        this->idx_ = 1;
    }

    lst_.append(Node(key, Type::key));
    lst_.append(val);

    if (root > 0) {
        lst_[root].size_++;
    }
    if (prev > 0) {
        const auto offset = u32(xpos - prev);
        lst_[prev - 0].next_ = offset;
        lst_[prev - 1].next_ = offset;
    }
    lst_[0].size_ += key.count() + u32(val.size());
    return xpos;
}

NMS_API void NodeEx::format(String& buf, StrView fmt) const {
    if (fmt == "json") {
        json::formatImpl(buf, *this, fmt);
    }
    else if (fmt == "xml") {
        xml::formatImpl(buf, *this, fmt);
    }
    else {
        io::log::error("nms.serialization.Tree.Format: unknow format type '{}'", fmt);
    }
}

}

