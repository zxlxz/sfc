#include <nms/test.h>
#include <nms/serialization/dom.h>

namespace nms::serialization
{

NMS_API XDOM XDOM::operator[](u32 k) const {
    if ( type() != Type::array) {
        NMS_THROW(EUnexpectType{ Type::array, type() });
    }

    const auto n = count();
    if (k >= n) {
        NMS_THROW(EUnexpectElementCount{ k + 1, n });
    }

    // find index
    auto itr = begin();
    for (auto i = 0u; i < k; ++i) {
        ++itr;
    }
    return *itr;
}

NMS_API XDOM XDOM::operator[](u32 k) {
    auto& node_list = *pnodes_;

    if (node_list.count() == 0) {
        node_list.append(DOM{ Type::null, 0 });
        node_list.append(DOM{ Type::array });
        this->index_ = 1;
    }
    else if (type() != Type::array) {
        NMS_THROW(EUnexpectType{ Type::array, type() });
    }

    // find index
    auto itr = begin();
    for (auto i = 0u; i < k; ++i) {
        // out of range, insert new val
        if (i >= count()) {
            auto pnode = add(index_, itr.idx_, DOM());
            (void)pnode;
        }
        ++itr;
    }
    return *itr;
}

NMS_API XDOM::Iterator XDOM::find(StrView expect) const {
    if (type() != Type::object) {
        NMS_THROW(EUnexpectType{ Type::object, type() });
    }

    auto n      = count();
    auto itr    = begin();
    for (u32 i = 0; i < n; ++i, ++itr) {
        auto key = itr.key();
        if (key == expect) {
            return { pnodes_, itr.idx_ };
        }
    }
    return { pnodes_, 0 };
}

NMS_API XDOM XDOM::operator[](StrView key) const {
    auto itr = find(key);
    if (itr != end()) {
        return *itr;
    }

    NMS_THROW(EKeyNotFound{ key });
}


NMS_API XDOM XDOM::operator[](StrView key) {
    auto& node_list = *pnodes_;

    if (node_list.count() == 0) {
        node_list.append(DOM{ Type::null, 0 });
        node_list.append(DOM{ Type::object });
        this->index_ = 1;
        auto idx = add(1, 0, key, DOM{ Type::null });
        return { pnodes_, idx };
    }

    auto& v = val();
    if (type() == Type::null) {
        v.type_ = Type::object;
    }
    else if (type() != Type::object) {
        NMS_THROW(EUnexpectType{ Type::object, type() });
    }

    auto n      = v.count();
    auto itr    = begin();
    auto last   = itr.idx_;

    for (u32 i = 0; i < n; ++i, ++itr) {
        last = itr.idx_;
        if (itr.key() == key) {
            return { pnodes_, itr.idx_ };
        }
    }

    // not find, insert new key/val
    auto pval = add(index_, last, key, DOM(Type::null));

    return XDOM{ pnodes_, pval };
}

NMS_API i32 XDOM::add(i32 root, i32 prev, const DOM& val) {
    auto& node_list = *pnodes_;

    auto xpos = i32(node_list.count());

    if (xpos == 0) {
        node_list.append(DOM{ Type::null, 0 });
        this->index_ = 1;
        ++xpos;
    }
    node_list.append(val);

    if (root > 0) {
        node_list[root].size_ += 1;
    }
    if (prev > 0) {
        node_list[prev].next_ = xpos - prev;
    }
    if (val.type() == Type::string) {
        node_list[0].size_ += u16(val.size());
    }
    return i32(xpos);
}

NMS_API i32 XDOM::add(i32 root, i32 prev, const StrView& key, const DOM& val) {
    auto& node_list = *pnodes_;

    auto xpos = i32(node_list.count() + 1);
    if (xpos == 1) {
        node_list.append(DOM{ Type::null, 0 });
        this->index_ = 1;
    }

    node_list.append(DOM(key, Type::key));
    node_list.append(val);

    if (root > 0) {
        node_list[root].size_++;
    }
    if (prev > 0) {
        const auto offset = xpos - prev;
        node_list[prev - 0].next_ = offset;
        node_list[prev - 1].next_ = offset;
    }
    node_list[0].size_ += DOM::Tsize(key.count() + u32(val.size()));
    return xpos;
}

}

