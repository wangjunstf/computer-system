# GDB使用教程

假设当前目录下共有以下3个文件，接下来会利用以下代码来演示GDB的调试过程。

├── main.cpp

├── tool.h

└── tools.cpp

文件内容分别为：

tool.h：函数声明

```cpp
void greeting();
int sum(int a,int b);
int sum(int a, int b);
```



tools.cpp：函数定义

```cpp
#include <iostream>
#include "tool.h"

using namespace std;

void greeting(){
    cout<<"Hello World"<<endl;
}

int sum(int a, int b){
    return a+b;
}
```



main.cpp：主函数

```cpp
#include <iostream>
#include <string.h>
#include "tool.h"
using namespace std;

#include <iostream>
#include <string.h>
#include "tool.h"
using namespace std;

void greeting();
int main(int argc, char* argv[]){
    int s = 0;
    int m=0;
    int n=0;
    for(int i=1; i<=atoi(argv[1]); i++){
        s+=i;
        m++;
        n++;
    }
    printf("sum=%d\n",s);
    greeting();
    cout<<sum(12,12)<<endl;
    print_num();
    return 0;
}
```

以上代码只是为了演示之用，并无特别之处。

编译方式

```shell
// -g 添加调试信息    -Wall 输出所有警告信息，例如定义了从未使用过的变量
$ gcc main.cpp -o main -g -Wall  
```



## 1、启动

```shell
gdb main
```



## 2、设置和获取参数

有时需要在执行程序时输入额外的参数，例如：像下面这样执行程序test

```shell
$ ./main 5
```

调试时，往往直接执行`$ gdb main`

那么怎么在gdb中输入该程序的参数呢？

```shell
(gdb) set args 5
```

还可以查看程序的参数

```shell
(gdb) show args
Argument list to give program being debugged when it is started is "5".
```



## 3、查看代码

GDB提供了两种查看源码的方式，分别是**根据行号**和**函数名**查看。除了可以查看本文件源码，还可以查看其他文件的源码。

### 3.1 本文件

本文件指的是该程序对应的main函数所在文件，即main.cpp

```shell
(gdb) l        # 每执行1次显示10行，再执行1次显示次10行
(gdb) l 15     # 显示第15行，此时会将15行显示在屏幕窗口中央，方便查看前后的代码
(gdb) l main   # 显示本文件的main函数
```



### 3.2 其他文件

共同编译的所有文件中，除了main函数所在文件的其它文件。在这里，除了main.cpp即tools.cpp

```shell
(gdb) l tools.cpp:15       		# 在tools.cpp中，显示第15行附近的代码
(gdb) l tools.cpp:sum      		# 在tools.cpp中，查看sum函数的代码
```



### 3.3 设置和获取显示行数

```shell
(gdb) show list    				# 显示行数
(gdb) set list 20  				# 设置行数
```



## 4、断点

可以在一次调试中设置1个或多个断点，下一次只需让程序自动运行到设置断点位置，便可在上次设置断点的位置中断下来，极大的方便了操作，同时节省了时间。

可以根据行号，函数名设置断点，还可根据条件设置断点(一般用于循环中)

### 4.1 本文件

```shell
(gdb) b 10              # 将第10行设置为断点
(gdb) b main            # 将main函数设置为断点
(gdb) l                 # 可以看到在main.cpp中含有greeting函数的声明
11
12	void greeting();
13	int main(int argc, char* argv[]){
14	    int s = 0;
15	    int m=0;
16	    int n=0;
17	    for(int i=1; i<=atoi(argv[1]); i++){
18	        s+=i;
19	        m++;
20	        n++;
(gdb) b greeting        # 此时设置的断点并不是函数的声明处，而是函数的定义处，greeting函数定义在tools.cpp文件中
Breakpoint 1 at 0xaa2: file tools.cpp, line 13.
```



### 4.2 其他文件

```shell
b tools.cpp:12            # 将tools.cpp的第12行设置为断点
b tools.cpp:sum           # 将tools.cpp的sum函数设置为断点
```



### 4.3 设置条件断点

条件断点一般用于循环中

本文件

```shell
# 设置i==2时，第18行为断点
# 行号必须在变量的作用域范围内
(gdb) b 18 if i==2                              
Breakpoint 1 at 0x9d8: file main.cpp, line 18.
```



其他文件

```shell
# 设置tools.cpp文件内，i==5时，第22行为断点
# 22必须在i的作用域范围
(gdb) b tools.cpp:22 if i==5                  
Breakpoint 2 at 0xafb: file tools.cpp, line 22.
```



### 4.4 查看和删除断点

```shell
(gdb) i b            # 显示所有断点
Num     Type           Disp Enb Address            What
1       breakpoint     keep y   0x00000000000009a4 in main(int, char**) at main.cpp:14
2       breakpoint     keep y   0x00000000000009ec in main(int, char**) at main.cpp:22
3       breakpoint     keep y   0x0000000000000a39 in main(int, char**) at main.cpp:25

d 1                  # 删除第1个断点                               
```



### 4.5 设置断点无效和有效

```shell
dis 2               # 将第2个断点设置为无效
ena 2               # 将第2个断点设置有效     
```



## 5、运行

有两种运行方式，一种是从主函数开始运行，一种是运行到第1个断点处

### 5.1 从主函数运行

程序从main函数开始

```
(gdb) run
```



### 5.2 运行到第1个断点处

程序停在第一个断点处

```
(gdb) start
```



### 5.3 执行流控制

```shell
c/continue         # 向下运行到下一个断点处
n/next             # 执行下一行代码，不进入调用的函数，直接返回结果
s/step             # 执行下一行代码，进入调用的函数
finish             # 跳出函数体
until    					 # 跳出当前循环 在执行完循环体内的最后一条语句之后执行 until，才可以跳出循环
```



### 5.4 打印变量

```shell
p  变量名                      # 打印变量值，可以答应在当前作用域之内的变量名
ptype 变量名                   # 打印变量类型
```



## 6、变量

### 6.1  自动变量操作

可以在每次执行时都打印该变量的值，常用于循环体中

```shell
display 变量名                 # 自动打印指定变量的值
i display         						# 查看所有设置自动打印的变量值
undisplay 编号     						 # 取消自动打印变量值
```



### 6.2 设置变量值

```shell
set var 变量名=变量值           # 可以临时改变该变量的值               
```



