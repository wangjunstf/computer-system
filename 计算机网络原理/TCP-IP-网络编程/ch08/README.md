# 第8章 域名及网络地址

## 1、域名系统

DNS(Domain Name System)是IP地址和域名进行相互转换的系统，其核心是DNS服务器。


## 1.1 什么是域名

通过IP地址可以让计算机很容易识别其他主机，但对人来说就显得不是很友好，因此就产生了域名这种东西，通过一串符号来对应一个IP，这样就更加方便人类识别和记忆，这串符号就是域名，例如www.google.com或www.apple.com



## 1.2 DNS服务器

当然计算机是不认识域名的，它只认识IP地址，那怎么来保存域名和IP地址的对应消息？就是通过DNS服务器，我们输入域名之后，计算机会访问DNS服务器(这些服务器的IP通常都是公开的)，获取其中保存的IP地址，然后原路返回，这样计算机就知道了该域名对应的IP地址。我们注册完一个域名后，一般系统会免费提供域名解析服务，即把域名与IP的映射关系存储在某些共有的DNS服务器中。



> 在生产环境中，一般不会改变服务器域名，但会相当频繁地改变服务器的IP地址。
>
> 查看域名对应IP ：
>
> ​	ping www.google.com
>
> 查看系统中的默认DNS服务器
>
> nslookup
>
> \>server



每台计算机都内置有一个DNS服务器，它并不知道网络上所有域名的IP地址信息，若该DNS服务器无法解析，则会依次询问其他DNS服务器，然后原路返回查询结果给查询主机。如下图所示工作：

<img src="https://wangjunblogs.oss-cn-beijing.aliyuncs.com/TCP-IP-Network-ch04/DNS%E5%92%8C%E8%AF%B7%E6%B1%82%E8%8E%B7%E5%8F%96IP%E5%9C%B0%E5%9D%80%E4%BF%A1%E6%81%AF.png" alt="DNS和请求获取IP地址信息" style="zoom: 33%;" />



根域名服务器（root name server）是互联网域名解析系统（DNS）中级别最高的域名服务器，负责返回顶级域的权威域名服务器地址。它们是互联网基础设施中的重要部分，因为所有域名解析操作均离不开它们。



图8-1展示了根据域名查找IP地址的过程，当一个服务器不存在时，会向更高层次DNS服务器查询，查询到时会原路返回，并在经过的服务器上形成缓存，下一次查询就可以直接通过缓存的数据获得目标IP地址。DNS就是这样一种层次化管理的一种**分布式**数据库系统。



## 2、IP地址和域名之间的转换

### 2.1 程序中有必要使用域名吗

关于这个问题，我们可以作这样一个假设？我们发布一款程序，程序里使用IP地址连接服务器，某一天我们需要更换服务器IP地址怎么办，修改源程序，把IP地址改为新IP地址，然后编译并发布，这时候通知用户需要升级我们的程序。有没有可能既能改变IP地址，又不让用户重新下载安装？

答案是使用域名，因为域名是动态绑定的，可以随时改为其他IP地址，这样程序里通过域名，就可以访问最新的IP地址。



### 2.3 利用域名获取IP地址

```c
#include <netdb.h>

struct hostent * gethostbyname(const char * hostname);
//成功时返回hostent结构体地址，失败时返回NULL指针
```

hostent结构体的具体内容

```c
struct hostent
{
  char* h_name;           //保存域名
  char** h_aliases;				//可以通过多个域名访问同一主页，同一个IP可以绑定多个域名，因此除官方域名外，还可以指定其他域名
  int h_length;           //IP地址长度，若为IPv4地址则是4字节，若为IPv6则为16字节
  char** h_addr_list;     //保存域名对应的IP地址，同一个域名可以对应多个IP地址，用于负载均衡
}
```

结构可以用下图表示

<img src="https://wangjunblogs.oss-cn-beijing.aliyuncs.com/TCP-IP-Network-ch04/hostent%E7%BB%93%E6%9E%84%E4%BD%93%E5%8F%98%E9%87%8F.png" alt="hostent结构体变量" style="zoom:33%;" />



现用以下程序获取域名的IP地址：gethostnamebyname.c

#### 源码

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

void error_handling(char *message);

int main(int argc, char* argv[]){
    struct hostent *host;
    if(argc!=2){
        printf("Usage: %s <addr>\n",argv[0]);
        exit(1);
    }

    host = gethostbyname(argv[1]);
    if(!host){
        error_handling("gethostbyname() error");
    }

    printf("Official name: %s \n",host->h_name);

    for(int i=0; host->h_aliases[i]; ++i){
        printf("Aliases %d: %s\n",i+1, host->h_aliases[i]);
    }

    printf("Address type: %s \n",(host->h_addrtype==AF_INET)?"AF_INET":"AF_INET6");

    for(int i=0; host->h_addr_list[i]; ++i){
        //  inet_ntoa() 将in_addr结构体中保存的地址转换为网络字节序iP地址
        printf("IP ADDR %d %s\n",i,inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
    }

    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
```

#### 编译运行

```shell
$ gcc gethostbyname.c -o ./bin/gethostbyname
$ ./bin/gethostbyname www.baidu.com
Official name: www.a.shifen.com 
Aliases 1: www.baidu.com
Address type: AF_INET 
IP ADDR 0 110.242.68.3
IP ADDR 1 110.242.68.4
```



### 2.4 利用IP地址来获取域名

```c
#include <netdb.h>

struct hostent * gethostbyaddr(const char * addr, socklen_t len, int family);
/*
	addr 含有IP地址信息的in_addr结构体指针。 为了同时传递IPv4地址之外的其他信息，该变量的类型声明为char的指针
	
	len 向第一个参数传递的地址信息的字节数，IPv4为4，IPv6为16
	
	family 传递地址族信息，IPv4为AF_INET，IPv6时为AF_INET6
*/
```

用以下程序获取IP的域名: gethostbyaddr.c

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>



```

