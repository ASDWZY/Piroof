# Piroof中文文档
Written by Festa

[主页](../Piroof中文文档.md)

[下一节 Piroof对象](./PiroofObjects.md)

## Piroof特性与代码示例

- Piroof具有2种模式，命令行模式和文件模式

- Piroof逐行解释，不使用';'分隔代码
- Piroof的注释写在'#'符号后
```
print("Hello world from Piroof") # 这是注释
```


### Piroof命令行模式

- 双击运行Piroof.exe即可弹出Piroof的命令行，进入命令行模式
- 在">>>"后输入代码，在新的一行会输出结果

```
>>>print("这是命令行模式")
这是命令行模式
>>>
```

- 在命令行模式中，如果返回值是void则不会输出，可以使用print函数打印
```
>>>void
>>>print(void)
void
>>>
```

### Piroof文件模式
- 用记事本编写xxx.pir
- 在Piroof.exe的路径下打开cmd，输入.\Piroof.exe xxx.pir即可运行xxx.pir

```
# example.pir
print("这是文件模式")
# out: 这是文件模式
```

```cmd
.\Piroof.exe example.pir
```

### Piroof 内存
- Piroof使用内存池，内存申请和释放效率高，具有垃圾自动回收机制

```
symbol x,y # 创建代数对象符号x,y
const a=123.123，b=true # 创建常量对象a,b
delete x,y,a # 删除对象
# 程序退出时删除所有对象
```

### Piroof expr
- Piroof提供了expr类进行数学操作

```
symbol x,y
assign e=(x+2)^2
print(expand(e)) # 表达式展开并标准化
# out: x^2+4x+4
```