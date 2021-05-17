# 第 17 章 优于select的epoll

## 1、epoll理解及应用

### 1.1 基于select的 I/O 复用技术慢的原因

从代码中可以分析其中的不合理的设计：

* 调用select函数后常见的针对所有文件描述符循环语句
* 每次调用select函数时都需要向该函数传递监视对象信息

表面上看好像是循环语句拖慢了select的运行效率，更大的障碍是每次传递监视对象信息。应用程序向操作系统传递数据将对程序造成很大的负担，而且无法通过优化代码解决，因此它将成为性能上的致命弱点。



可不可以仅向操作系统传递一次监视对象，监视范围或内容发生变化时只通知发生变化的事项？

各个系统的支持方式不一样。Linux的支持方式为epoll



### 1.2 select 的优点

大部分操作系统都支持select，select适用于以下两种情况：

* 服务器端接入者少
* 程序应具有兼容性



### 1.3 实现epoll时必要的函数和结构体

#### epoll具有以下优点：

* 无需编写以监视状态变化为目的的针对所有文件描述符的循环语句
* 调用对应于select函数的epoll_wait函数时无需每次传递监视对象的信息



epoll服务器端实现中需要的3个函数：

* epoll_create：创建保存epoll文件描述符的空间
* epoll_ctl: 向空间注册并注销文件描述符
* epoll_wait：与select函数类似，等待文件描述符发生变化



为了添加和删除监视对象文件描述符，select方式中需要FD_SET，FD_CLR函数，但在epoll方式中，通过epoll_ctl函数请求操作系统完成。



在epoll方式中，通过如下结构体epoll_event将发生变化的文件描述符单独集中到一起。

```c
struct epoll_event{
  __uint32_t events;
  epoll_data_t data;
}

typedef union epoll_data{
  void *ptr;
  int fd;
  __uint32_t u32;
  __uint64_t u64;
}epoll_data_t;
```



声明足够大的epoll_event结构体数组后，传递给epoll_wait函数时，发生变化的文件描述符信息将被填入该数组。



#### epoll_create

epoll是从Linux 2.5.44版内核开始引入的。

查看系统内核版本命令如下：

```shell
$ cat /proc/sys/kernel/osrelease
```



函数定义

```c
#include <sys/epoll.h>

int epoll_create(int size);
// 成功时返回epoll文件描述符，失败时返回-1

// size epoll实例的大小
```



调用epoll_create创建的文件描述符保存空间称为"epoll例程"。

Linux 2.6.8之后的内核完全忽略传入epoll_create函数的size参数，因为内核会根据情况调整epoll例程的大小。



epoll_create函数创建的资源与套接字相同，也由操作系统管理。该函数返回的文件描述符主要为了区分epoll例程。与其他文件描述符相同，需要终止时调用close函数。



#### epoll_ctl

用于在epoll例程内部注册监视对象文件描述符。

```c
#include <sys/epoll.h>

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
// 成功时返回0，失败时返回-1

/*
	epfd 用于注册监视对象的epoll例程的文件描述符
	op 用于指定监视对象的添加，删除或更改操作
	fd 需要注册的监视对象文件描述符
	event 监视对象的事件类型
*/
```



举例：

epoll_ctl(A，EPOLL_CTL_ADD, B, C);

第2个参数：EPOLL_CTL_ADD表示”添加“

含义：epoll例程A中注册文件描述符B，主要目的是监视参数C中的事件



epoll_ctl(A，EPOLL_CTL_DEL, B, NULL);

第2个参数：EPOLL_CTL_DEL表示”删除“

含义：从epoll例程中删除文件描述符B



epoll_ctl函数的第2个参数可以传递的形参有以下3种：

* EPOLL_CTL_ADD：将文件描述符注册到epoll例程。
* EPOLL_CTL_DEL：从epoll例程中删除文件描述符。
* EPOLL_CTL_MOD：更改注册的文件描述符的关注事件发生情况。



##### 调用示例

```c
struct epoll_event event;
...
event.events=EPOLLIN; // 发生需要读取数据的情况时
event.data.fd=sockfd;
epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event);
...
```



epoll_event的成员events中可以保存的常量及所指的事件类型：

* EPOLLIN：需要读取数据的情况。
* EPOLLOUT：输出缓冲区为空，可以立即发送数据的情况。
* EPOLLPRI：收到OOB(out-of-date)数据的情况
* EPOLLRDHUP：断开连接或半关闭的情况，这在边缘触发方式下非常有用。
* EPOLLERR：发生错误的情况。
* EPOLLET：以边缘触发的方式得到事件通知。
* EPOLLONESHOT：发生一次事件后，相应文件描述符不再收到事件通知。因此需要向epoll_ctl函数的第二个参数传递EPOLL_CTL_MOD，再次设置事件。

可以通过位或'|'运算同时传递多个上述参数。



#### epoll_wait 

```c
#include <sys/epoll.h>

int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout);
// 成功时返回发生事件的文件描述符数，失败时返回-1

/*
	epfd 表示事件发生监视范围的epoll例程的文件描述符。
	events 保存发生事件的文件描述符集合的结构体地址值
	maxevents 第二个参数中可以保存的最大事件数
	timeout 以1/1000秒为单位的等待时间，传递-1时，一直等待直到事件发生。
*/
```

epoll_wait函数第二个参数需要动态分配。

##### 调用示例

```c
int event_cnt;
struct epoll_event *ep_events;
...
ep_events = malloc(sizeof(struct epoll_event)*EPOLL_SIZE);  //EPOLL_SIZE是宏常量
...
event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
...

```



## 2、基于epoll的回声服务器端

