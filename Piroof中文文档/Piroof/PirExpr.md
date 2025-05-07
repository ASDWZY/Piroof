# Piroof中文文档
Written by Festa

[主页](../Piroof中文文档.md)

## Piroof表达式对象
表达式，例如x^2+1,(2x+y-y)^2等，它们在Piroof中不会自动展开或化简或标准化，但Piroof会去除多余的括号，注意请使用小括号

一个代数符号同样属于表达式对象

表达式分为常量表达式和变量表达式，这里的常量和变量是数学上的；

使用isconstexpr判断是否是常量表达式，如果返回true则是，返回false则是变量表达式，返回notsure则说明输入不是表达式类型
- 使用symbol创建变量代数符号
```
>>>symbol x,y,z
>>>x
x
>>>x^2+1
x^2+1
>>>isconstexpr(x)
false
>>>isconstexpr(x^2+1)
false
>>>
```

- 使用const symbol创建常量代数符号
```
>>>symbol x,y,z
>>>const symbol a,b,c
>>>a
a
>>>a*x+b
a*x+b
>>>isconstexpr(a)
true
>>>isconstexpr(a*x+b)
false
```

### 不只是数学符号
- 表达式对象同样可以表示逻辑运算，在进行逻辑运算时与数值运算时相同，返回一个表达式而不是逻辑结果
```
>>> symbol x
>>> x=x+1
x=x+1
>>> x<5
x<5
>>>
```

### 4种表达式对象分类
- 对于常量表达式和变量表达式，他们的对象可以是常量对象或非常量对象，可分为以下4种表达式
```
>>> symbol x       #对象是常量的 变量表达式
>>> const symbol a #对象是常量的 常量表达式
>>> assign e=x     #对象是变量的 变量表达式
>>> assign t=a     #对象是变量的 常量表达式
```

- 可以发现，在不进行赋值的情况下，表达式对象的计算和操作结果得到的都是对象是常量的表达式，即
```
>>> symbol x
>>> const symbol a
>>> isconst(x)
true
>>> isconst(x*x+a)
true
>>> isconst(x=x)
true
```

### 创建替换函数表达式
可以通过以下方式创建一个 替换函数表达式
```
>>> f(x)=x^2+x # 这里的x在之前是否被创建与此操作无关
>>> symbol y
>>> f(y)
y^2+y
>>> f(1)
1^2+1
>>> f
<expr-function>
>>> type(f)
expr
```

### 常用内置函数
- expand函数可以把表达式展开并标准化

```
symbol x,y,z
assign e=(x+y+z)^2
print(expand(e))
# out: x^2+2x*y+2x*z+y^2+2y*z+z^2
```