#pragma once

#include "sfc/core/mod.h"
#include "sfc/core/ptr.h"
#include "sfc/core/mem.h"
#include "sfc/core/num.h"
#include "sfc/core/ops.h"
#include "sfc/core/str.h"
#include "sfc/core/fmt.h"
#include "sfc/core/slice.h"
#include "sfc/core/trait.h"

#include "sfc/core/tuple.h"
#include "sfc/core/option.h"
#include "sfc/core/result.h"
#include "sfc/core/variant.h"

#include "sfc/core/future.h"

namespace sfc {

using ops::_;
using ops::Range;

using str::Str;

using slice::Slice;

using option::Option;
using result::Result;

using tuple::Tuple;
using variant::Variant;

using future::Future;
}  // namespace sfc
