
# unit test

![nms.test](https://github.com/lumpyzhu/nmscc/blob/master/doc/nms.test.gif)

使用单元测试是很简单的，无论是库，还是可执行文件中，都可以使用
```cpp
nms_test(my_test) {
    // codes here
}
```
创建一个测试。

如果出现异常，则测试失败，也可以使用断言来产生异常，比如：
```cpp
nms::assert_eq(a, b);       // 判断是否相等
nms::assert_neq(a, b);      // 判断是否不等
```

之后，这个动态库或可执行程序就包含了测试信息。
使用nms.test对它进行单元测试


```shell
% 全部测试
# nms.test  my_library.dll
   ....
   
% 只包含my_namespace下面的测试   
# nms.test my_library.dll my_namespace::
...

% 排除my_namespace下面的测试
# nms.test my_library.dll -my_namespace::my_class 
```

