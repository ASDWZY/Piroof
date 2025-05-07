# Piroof中文文档
Written by Festa

[主页](../Piroof中文文档.md)

[下一节 Piroof表达式对象](./Piroof/PirExpr.md)

## Piroof内置类型
- 所有类型都是Piroof对象

- 通过type函数获取对象的类型

```
>>>type(void)
voidtype
>>>type(123)
number
>>>type("123")
string
>>>type(true)
logic
>>>type(print)
<builtin-function>
>>>type(number)
type
>>type(type)
type
```

可以看到Piroof的内置类型，常用的有：
- int, 整型，无大小限制
- float，浮点数，与C++中的double类型相同，有误差
- number，有理数，有大小限制和精度限制，使用分数计算，无误差
- string，字符串，支持中文
- logic，4值逻辑状态，包括true,false,notsure,contra
- expr，表达式
- type，类型（的类型）
- Exception，异常类型，用于引发异常

通过类型函数可以把一个对象转化成目标类型的对象
- 把string对象转化成number对象
```
>>>number("123")
123
```
- 把number对象转化成expr对象并相加
```
>>>expr(1.5)+expr(1.5)
3/2+3/2
```

