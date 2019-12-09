#pragma once

#include "rc/core.h"

namespace rc::fs {

struct SeekFrom {

  enum class Tag {
    Start,
    Current,
    End,
  };

  Tag _whence;
  i64 _offset;

  auto Start(u64 offset) noexcept -> SeekFrom {
    return SeekFrom{Tag::Start, i64(offset)};
  }

  auto Current(i64 offset) noexcept -> SeekFrom {
    return SeekFrom{Tag::Current, i64(offset)};
  }

  auto End(i64 offset) noexcept -> SeekFrom {
    return SeekFrom{Tag::End, i64(offset)};
  }
};

}  // namespace rc::fs
