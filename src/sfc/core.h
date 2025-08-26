#pragma once

#include "sfc/core/fmt.h"
#include "sfc/core/future.h"
#include "sfc/core/mod.h"
#include "sfc/core/num.h"
#include "sfc/core/ops.h"
#include "sfc/core/ptr.h"
#include "sfc/core/result.h"
#include "sfc/core/slice.h"
#include "sfc/core/str.h"
#include "sfc/core/variant.h"

namespace sfc {
using str::Str;
using slice::Slice;

using ops::Range;
using option::Option;
using result::Result;

using tuple::Tuple;
using variant::Variant;

using future::Future;
}  // namespace sfc
