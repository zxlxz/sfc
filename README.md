# nmscc
[![Travis](https://img.shields.io/travis/lumpyzhu/nmscc.svg)](https://travis-ci.org/lumpyzhu/nmscc)
[![AppVeyor](https://img.shields.io/appveyor/ci/lumpyzhu/nmscc.svg)](https://ci.appveyor.com/project/lumpyzhu/nmscc)
[![license](https://img.shields.io/github/license/lumpyzhu/nmscc.svg)]()

NMSCC: NMS Common C++ Library
------------------------------


### What is `nmscc`?
nmscc is a library of C++14 components designed for for scientific computation.

nmscc是一个为科学计算设计的C++14库，关注运行效率与开发效率，并在商业科学计算领域己有了一些应用。

nmscc涉及矩阵代数，并行计算，序列化等方面，也包含了STL,BOOST的一些概念，但这里从设计的统一性考虑，没有依赖STL,BOOST。

## Dependencies
  nmscc 支持windows/linux/macos平台


  以下环境经过测试可以正常使用:

- Windows 10(x64)
  1. Visual Studio 2015(with update 3)
  2. Visual Studio 2017
- Ubuntu 16.04~16.10
  1. GCC-6.x
  2. GCC-7.x
- MacOS 10.12~10.13
  1. Apple CLANG 8.2~8.3
  2. GCC-7.x

```
git clone --recursive https://github.com/lumpyzhu/nmscc.git
```
  Windows平台，可以打开nms/nms.sln工程，进行编译调试。
  Linux/Mac平台，可以使用make或xmake编译。
```
  cd nms
  export CXX=g++-7    # use g++
  export CXX=clang++  # use clang
  make -j9
  make test           # run unittest
```

### nms.core
- [type](/docs/core/type.md) 类型支持
- [view](/docs/core/view.md) 多维向量视图
- [list](/docs/core/list.md) 类似c++ std::vector
- [string](/docs/core/string.md) 字符串

### nms.io
- [path](/docs/io/path.md) 文件系统
- [file](/docs/io/file.md) 文件

### nms.math
- [complex](/docs/math/complex.md) 复数
- [array](/docs/math/array.md) 多维数组
- [lazy-expression](/docs/math/lambda.md) 表达式计算

### nms.cuda
- [cuda](/docs/cuda/cuda.md) cuda
- [array](/docs/cuda/array.md) 数组
- [texture](/docs/cuda/array.md) 纹理
- [nms.compiler](/docs/nms.cuda.compiler.md) 编译器(nvrtc封装)

### nms.thread
- [thread](/docs/thread/thread.md) 线程
- [mutex](/docs/thread/mutex.md) 锁
- [semaphore](/docs/thread/semaphore.md) 信号量
- [condition variable](/docs/thread/condvar.md) 条件变量
- [task](/docs/thread/task.md) 任务/工作流

### nms.test
- [test](/docs/nms/test.md) 单元测试

### nms.serialization
- [tree](/docs/serialization/tree.md) 属性树
- [json](/docs/serialization/json.md) json支持
- [xml](/docs/serialization/xml.md) xml支持

### nms.util
- [ArrayList](/docs/util/arraylist.md) 动态数组
- [Library](/docs/util/library.md) 动态库
- [RingBuf](/docs/util/ringbuf.md) 环形缓冲区
- [StackTrace](/docs/util/stacktrace.md) 栈回溯
- [System](/docs/util/system.md) 系统调用
