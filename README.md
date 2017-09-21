# NMSCC: NMS Common C++ Library

[![Travis](https://img.shields.io/travis/lumpyzhu/nmscc.svg)](https://travis-ci.org/lumpyzhu/nmscc)
[![AppVeyor](https://img.shields.io/appveyor/ci/lumpyzhu/nmscc.svg)](https://ci.appveyor.com/project/lumpyzhu/nmscc)
[![license](https://img.shields.io/github/license/lumpyzhu/nmscc.svg)](https://www.gnu.org/licenses/lgpl.html)

## Introduction

nmscc is a library of C++14 components designed for for scientific computation.

nmscc是一个为科学计算设计的C++14库，关注运行效率与开发效率，并在商业科学计算领域己有了一些应用。

nmscc涉及矩阵代数，GPU/CPU并行计算，序列化，单元测试等。

## Dependencies

  nmscc 支持`windows`/`linux`/`macos`平台，需要编译器支持C++14。

  以下环境经过测试可以正常使用:

- Windows 10(x64)
  1. Visual Studio 2015(with update 3)
  1. Visual Studio 2017

- Linux
  1. GCC-6.x
  1. GCC-7.x

- MacOS
  1. Apple CLANG 8.2~8.3
  1. GCC-7.x

## Usage

下载代码

``` bash
git clone --recursive https://github.com/lumpyzhu/nmscc.git
```

### Windows

  用Visual Studio打开nms/nms.sln工程，进行编译调试。

### Macos

``` bash
  cd nms
  make -j9
  make test           # run unittest
```

### Linux

  有的Linux默认的C++编译器是GCC-5.x，需要升级到gcc-6，gcc-7，或使用clang

  如果有多个版本的编译器，请先设置CXX环境变量

``` bash
  export CXX=g++-7    # select g++-7
  cd nms
  make -j9
  make test           # run unittest
```

## docs

- [中文文档](https://github.com/lumpyzhu/nmscc.docs/blob/master/index.md)

## Conduct

- [行为准则](CODE_OF_CONDUCT.md)

## Contributing

- [参与开发](Contributing.md)
- 加入Q群讨论组：596765241
