
# unit test

![nms.test](https://github.com/lumpyzhu/nmscc/blob/master/doc/nms.test.gif)

如果有以下代码:
```cpp
namespace my_ns
{
class T
{
public:
   void func_1();
   void func_2();
private:
   //...
};
}
```
想对此做单元测试，方法如下:
```cpp
#include <nms/test.h>        // 包含单元测试头文件

namespace my_ns
{
class Foo
{
public:
   void func_1();
   bool func_2();
private:
   //...
};

nms_test(Test1) {          // 创建一个测试,命名为Test1
   Foo t;
   t.func_1();             // 调用func_1，如果没有发生异常，认为测试成功
}

nms_test(Test2) {
   Foo t;
   auto ret = t.func_2();
   nms::assert_eq(ret, true); // 也可以用assert进行断言测试
}

}
```

之后，编译这个项目，会生成动态库或者可执行程序。
使用nms.test对它进行单元测试，下面假设生成的动态库为my_lib.dll


```shell
% 全部测试
# nms.test  my_lib.dll
   ....
   
% 请注意到， Test1和Test2在my_ns名字空间下，如果只想开启这个名字空间下的单元测试：
# nms.test my_lib.dll my_ns::
...

% 排除my_ns下面的测试
# nms.test my_lib.dll -my_ns::
```

