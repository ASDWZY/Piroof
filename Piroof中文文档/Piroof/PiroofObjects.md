# Piroof中文文档
Written by Festa

[主页](../Piroof中文文档.md)

[下一节 Piroof内置类型](./Piroof/PiroofTypes.md)

## Piroof Objects
在Piroof中，除了关键字(key words)和运算符(operators)，所有皆对象；一个对象有名称、地址、类型和值等；

Piroof对象在C++中的伪代码：
```C++
struct PiroofObject{
    bool constant;
    bool deleted;
    std::string name;
    //...
}
PiroofObject* a=new PiroofObject;//地址为a
a->constant=true;
a->deleted=false;
a->name="a";
//...
```

Piroof的对象可以分为常量对象和非常量对象,所有的内置对象都是常量对象；常量对象是不可被更改的，非常量对象可以通过assign关键字<br><br>

创建一个名称是a，类型是number，值是1的非常量对象
```
var a=1
```

或者
```
assign a=1
```

更改a的值为2
```
assign a=2
```


创建一个名称是b，类型是number，值是1的常量对象
```
const b=1
```
这里assign和var都可以从无到有创建对象，区别在于如果创建的变量名称已经存在，使用var会导致重定义异常，但使用assign不会

### 同时操作多个对象

- 在同一行中使用','分隔

创建对象
```
var a=1,b,c="asd"
```

或者
```
assign a=1,b,c="asd"
```

删除对象
```
delete a,b,c
```

### 等于号
- 在Piroof中，为了让代码更加数学标准化，'='用于判断2个对象的值是否相等，而不是赋值

```
>>> var a=1
>>> a=2
false
>>> a=1
true
>>>
```

- 当是expr对象时，'='的表达式会被返回，而不是判断表达式相等
```
>>> symbol x
>>>x=x+1
x=x+1
>>>
```

### 常用内置函数
- 使用print函数打印对象,可以使用<参数名>=<值>的方式传参;print函数返回void值
```
>>>var a=1.5
>>>print(a) # print(obj,end="\n") -> void
3/2
>>print(obj=a,end="")
3/2>>>
```

- 使用isconst函数判定对象是否为常量对象，true则是常量对象，false则是非常量对象
```
>>>var a=1;
>>>const b=a;
>>>isconst(a)
false
>>>isconst(b)
true
```