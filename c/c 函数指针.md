# c 函数指针

## 1、声明

`void (*funP)(args);`

funP 为函数指针名



## 2、初始化

`funP = &funName`

funName为函数名



## 3、用函数指针调用函数

 `(*funP)(200)`



## 4、代码示例

```c
#include <stdio.h>

void myFun(int x);       //声明一个函数，参数为一个int
void (*funP)(int);       //声明一个函数指针，参数为一个int

int main(int argc, char *argv[])
{

    funP = &myFun;       //初始化函数指针
    (*funP)(200);        //调用函数指针
    return 0;
}

void myFun(int x)
{
    printf("myFun: %d\n", x);
}
```



喜欢钻研，热爱学习。沉寂于技术的快乐，也痴迷于Lisp的魅力。渴望拥有自己的操作系统，设计自己的编程语言。知识没有尽头，学习没有止境，追随自己的内心，去做自己真正热爱的事情。