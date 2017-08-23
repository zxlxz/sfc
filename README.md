# nmscc
[![Travis](https://img.shields.io/travis/lumpyzhu/nmscc.svg)](https://travis-ci.org/lumpyzhu/nmscc)
[![AppVeyor](https://img.shields.io/appveyor/ci/lumpyzhu/nmscc.svg)](https://ci.appveyor.com/project/lumpyzhu/nmscc)
[![license](https://img.shields.io/github/license/lumpyzhu/nmscc.svg)]()

nmscc is a C++14/CUDA Template library for scientific computation:liner algebra(nd-array), unit test, string format, logging, json/xml serialization etc.

nmscc是一个C++14/CUDA的科学计算库，包含线性代数(多维数组)，单元测试，格式化输出，日志，json/xml序列化等等。

![nms.test](./doc/test/test.gif)

### nms.core
- [type](/doc/core/type.md) 类型支持
- [view](/doc/core/view.md) 多维向量视图
- [list](/doc/core/list.md) 类似c++ std::vector
- [string](/doc/core/string.md) 字符串

### nms.io
- [path](/doc/io/path.md) 文件系统
- [file](/doc/io/file.md) 文件

### nms.math
- [complex](/doc/math/complex.md) 复数
- [array](/doc/math/array.md) 多维数组
- [lazy-expression](/doc/math/lambda.md) 表达式计算

### nms.cuda
- [cuda](/doc/cuda/cuda.md) cuda
- [array](/doc/cuda/array.md) 数组
- [texture](/doc/cuda/array.md) 纹理
- [nms.compiler](/doc/nms.cuda.compiler.md) 编译器(nvrtc封装)

### nms.thread
- [thread](/doc/thread/thread.md) 线程
- [mutex](/doc/thread/mutex.md) 锁
- [semaphore](/doc/thread/semaphore.md) 信号量
- [condition variable](/doc/thread/condvar.md) 条件变量
- [task](/doc/thread/task.md) 任务/工作流

### nms.test
- [test](/doc/nms.test.md) 单元测试

### nms.serialization
- [tree](/doc/serialization/tree.md) 属性树
- [json](/doc/serialization/json.md) json支持
- [xml](/doc/serialization/xml.md) xml支持

### nms.util
- [ArrayList](/doc/util/arraylist.md) 动态数组
- [Library](/doc/util/library.md) 动态库
- [RingBuf](/doc/util/ringbuf.md) 环形缓冲区
- [StackTrace](/doc/util/stacktrace.md) 栈回溯
- [System](/doc/util/system.md) 系统调用
