#pragma once

#include "sfc/collections/hash/hash_set.h"
#include "sfc/collections/hash/hash_map.h"

namespace sfc {
template <class K, class V>
using Dict = collections::HashMap<K, V>;

template <class T>
using Set = collections::HashSet<T>;
}  // namespace sfc
