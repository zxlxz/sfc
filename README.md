NMSCC: NMS Common C++ Library
------------------------------
[![Travis](https://img.shields.io/travis/lumpyzhu/nmscc.svg)](https://travis-ci.org/lumpyzhu/nmscc)
[![AppVeyor](https://img.shields.io/appveyor/ci/lumpyzhu/nmscc.svg)](https://ci.appveyor.com/project/lumpyzhu/nmscc)
[![license](https://img.shields.io/github/license/lumpyzhu/nmscc.svg)]()

### What is `nmscc`?
nmscc is a library of C++14 components designed for for scientific computation.

nmscc是一个为科学计算设计的C++14库，关注运行效率与开发效率，并在商业科学计算领域己有了一些应用。

nmscc涉及矩阵代数，GPU/CPU并行计算，序列化，单元测试等。

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

## docs
[中文文档](https://github.com/lumpyzhu/nmscc.docs/blob/master/index.md)
