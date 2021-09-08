# c语言 static关键字作用

## 1、保持变量内容的持久

这一点和全局变量类似。存储在静态数据区的变量会在程序刚开始运行时就完成初始化，也是唯一的一次初始化。共有两种变量存储在静态存储区：全局变量和static变量。

例如：

```c
#include <stdio.h>

void f();          // 函数声明
int main()
{
    f();
    f();
    f();
    f();
    return 0;
}

void f(){          // 函数定义 
    static int i = 0;
    i++;
    printf("%d\n",i);    
}
```

编译运行

```shell
$ gcc static.c -o ./static
$ ./static 
1
2
3
4
```



## 2、控制变量或函数的可见范围

当我们同时编译多个文件时，所有未加static关键字的变量或函数定义都具有全局可见性。接下来用程序验证：

a.c

```c
#include <stdio.h>

int a = 1024;       // 定义并初始化一个全局变量a

void f(){           // 定义了一个函数
    printf("Hello World\n");
}
```

main.c

```c
#include <stdio.h>

extern int a;            // extern关键字显式指定变量a为声明，不是定义。如果不指定extern，当其他.c文件里没有定义a时，编译器就把a当作全局变量，并默认初始化为0
void f();         
int main(){
    printf("%d\n",a);
    f();
    return 0;
}
```



a.c中定义的函数f和变量a，因为未加static，所以在下列编译过程中具有全局可见：

```shell
$ gcc main.c a.c -o main  # 编译main.c a.c 并连接为可执行文件main
$ ./main
1024
Hello World
```



接下来使用static关键字，将变量a的可见访问限制在a.c中

a.c

```c
#include <stdio.h>

static int a = 1024;       // 定义并初始化一个静态变量

void f(){                  // 定义了一个函数
    printf("Hello World\n");
}
```

main.c

```c
#include <stdio.h>

extern int a;            // extern关键字显式指定变量a为声明，不是定义。如果不指定extern，当其他.c文件里没有定义a时，编译器就把a当作全局变量，并默认初始化为0
void f();         
int main(){
    printf("%d\n",a);
    f();
    return 0;
}
```

编译连接

```shell
$ gcc main.c a.c -o main
/tmp/ccjNRJod.o: In function `main':
main.c:(.text+0x6): undefined reference to `a'
collect2: error: ld returned 1 exit status
```

编译报错了，这和预期一致，因为在a.c中，变量a被声明为static，只在a.c中可见，在其他文件中是不可见的，所以在编译连接时，显示“a未定义”，**证明了static的作用：“封装”，当想让变量只在当前文件内有效时，可以使用staic关键字。**









