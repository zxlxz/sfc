# nmscc
an c++14 common library for scientific computing.

[![Build status](https://ci.appveyor.com/api/projects/status/ry9oa2mxrj8hk613/branch/master?svg=true)](https://ci.appveyor.com/project/lumpyzhu/nmscc/branch/master)
[![license](https://img.shields.io/github/license/lumpyzhu/nmscc.svg)]()

这是一个面向科学计算的C++库。
主要包含对多维数组的惰性表达式求值，数学库，及HPC(CUDA/OpenCL)支持，
及一些辅助功能，如单元测试，字符串格式化，Json/Xml序列化等。

因为C++STL的命名规范，设计理念和本库区别较大，所以这里没有使用STL。

![nms.test](https://raw.githubusercontent.com/lumpyzhu/nmscc/master/doc/nms.test.gif)

### nms.core
- type
- view
- list
- string

### nms.io
- path
- file

### nms.math
- lamda expression

### nms.hpc
- [cuda](/doc/nms.hpc.cuda.md)
- opencl

### nms.thread
- thread
- mutex
- semaphore
- condition variable
- task

### nms.test
- [test](/doc/nms.test.md)

### nms.serialization
- tree
- json
- xml

### nms.util

