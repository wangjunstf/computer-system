

# 第7章 优雅地断开套接字连接

TCP中的断开连接过程往往比连接过程更重要，连接过程往往不会出什么变数，但断开过程有可能发生预想不到的情况。只有掌握了半关闭，才能明确断开过程。

## 7.1 基于TCP的半关闭

### 单方面断开连接带来的问题

Linux的close函数意味着完全断开连接，完全断开不仅指无法传输数据，也无法接收数据。在某些情况下，通信一方调用close函数断开连接就显得不太优雅。

![单方面断开连接](https://wangjunblogs.oss-cn-beijing.aliyuncs.com/TCP-IP-Network-ch04/%E5%8D%95%E6%96%B9%E9%9D%A2%E6%96%AD%E5%BC%80%E8%BF%9E%E6%8E%A5.png)





### 套接字和流

两台主机通过套接字建立连接后进入可交换数据的状态，又称为"流形成的状态"。也就是把建立套接字后可交换数据的状态看作一种流。

![套接字中生成的两个流](https://wangjunblogs.oss-cn-beijing.aliyuncs.com/TCP-IP-Network-ch04/%E5%A5%97%E6%8E%A5%E5%AD%97%E4%B8%AD%E7%94%9F%E6%88%90%E7%9A%84%E4%B8%A4%E4%B8%AA%E6%B5%81.png)



本章讨论的"优雅地断开连接方式"只断开其中1个流，而非同时断开两个流。



### 针对优雅断开的shutdown函数

shutdown函数用来关闭套接字的一个流。

```c
#include <sys/socket.h>

int shutdown(int sock, int howto);
//成功时返回0，失败时返回-1

/*
	需要断开的套接字文件描述符
	howto 传递断开方式信息
*/
```



howto的可选值：

* SHUT_RD:  断开输入流
* SHUT_WR:  断开输出流
* SHUT_RDWR:  同时断开I/O流

### 为何需要半关闭

当服务器端向客户端发送一个文件时，发送完毕后，需要得到客户端的反馈以确认文件是否正确送达。对服务器端而言，没什么问题，它只要把文件发出即可。

但对于客户端而言，需要考虑到：每次从服务器端接收文件的一部分，接收多少次才能接收整个文件。针对这个问题，可以向客户端传递一个特殊符号EOF来表示文件的结束，但服务器端如何传输EOF，一个有效办法就是通过客户端接收函数的返回值。

> 断开输出流时向对方主机传输EOF。

服务器端只需断开输出流，这样就告诉客户端文件传输完毕，同时服务器端也能收到客户端接收成功的返回信息。



## 7.2 基于半关闭的文件传输程序

客户端向服务器端发出连接请求，连接成功后，服务器端将文件file_server.c发送给客户端，客户端接收成功后，给服务器端发送消息: "Thank you"。

### 源代码

file_server.c

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char* message);

int main(int argc, char* argv[]){
    int serv_sd,clnt_sd;
    FILE *fp;
    char buf[BUF_SIZE];
    int read_cnt;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz; //记录客户端地址结构体长度
    if(argc != 2){
        printf("Usage: %s <PORT>\n", argv[0]);
        exit(1);    
        //立即终止调用过程，属于该进程的所有文件描述符都被关闭，
        //并且该进程的任何子进程都由进程1(init)继承，并且向该进程的父进程发送SIGCHLD信号。
    }

    fp = fopen("file_server.c","rb");   //以二进制只读方式打开，该文件必须存在
    serv_sd = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    bind(serv_sd, (struct sockaddr *)&serv_adr, sizeof(serv_adr));
    listen(serv_sd,5);   //开启监听状态，等待队列数为5

    clnt_adr_sz = sizeof(clnt_adr);
    clnt_sd = accept(serv_sd, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

    while(1){
        read_cnt = fread((void *)buf,1, BUF_SIZE, fp);
        if(read_cnt<BUF_SIZE){
            write(clnt_sd, buf, read_cnt); //每次读取BUF_SIZE，当读取的字节数不足BUF_SIZE，说明已经读取完毕了
            break;
        }
        write(clnt_sd, buf,BUF_SIZE);
    }

    shutdown(clnt_sd,SHUT_WR);
    read_cnt = read(clnt_sd, buf, BUF_SIZE);
    buf[read_cnt] = 0;
    printf("%s\n",buf);
    fclose(fp);
    shutdown(clnt_sd, SHUT_RD);
    close(serv_sd);
    

    return 0;
}

void error_handling(char* message){
    fputs("message",stderr);
    fputc('\n',stderr);
    exit(1);
}
```



file_client.c

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30

void error_handling(char* message);

int main(int argc, char* argv[]){
    int sd;
    FILE *fp;

    char buf[BUF_SIZE];
    int read_cnt;
    struct sockaddr_in serv_adr;    //客户端地址结构体

    if(argc!=3){
        printf("Usage: %s <IP> <PORT>\n",argv[0]);
        exit(1);
    }

    fp = fopen("receive.dat","wb");  //以只写二进制方式创建一个文件并打开，若已存在同名文件，则丢弃该文件所有数据，并打开
    sd = socket(PF_INET, SOCK_STREAM,0);

    memset(&serv_adr, 0 , sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if(connect(sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1){
        error_handling("connect() error");
    }

    while((read_cnt = read(sd, buf, BUF_SIZE))!= 0){
        fwrite((void*)buf, 1, read_cnt, fp);
    }

    puts("receive file data");
    write(sd,"Thank you!",11);
    fclose(fp);
    close(sd);
    return 0;
}

void error_handling(char* message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}
```



### 编译运行

服务器端

```shell
$ gcc file_server.c -o file_server
$ ./file_server 9190
Thank you!
```



客户端

```shell
$ gcc file_client.c  -o file_client
$ ./file_client 127.0.0.1 9190
receive file data
```



执行完毕后，服务器端收到消息"Thank you!",  客户端所在文件夹多了新文件"receive.dat",并打印出"receive file data".



### 7.3 习题

（1）解释TCP中“流”的概念。UDP中能否形成流？请说明原因。

> 答：TCP中的流，就是服务器端与客户端建立连接后，形成两条流，分别为输入流和输出流。客户端的输出流对应服务器端的输入流，客户端的输出流对应服务器端的输出流。
>
> 不能，因为UDP不需要建立连接，通过完整数据包直接传输数据。

（2） Linux中的close函数或Windows中的closesocket函数属于单方面断开连接的方法，有可能带来一些问题。什么是单方面断开连接？什么情况下会出现问题？

> 答：会让服务器端/客户端都无法再发送数据和接收数据。当服务器/客户端需要向对方发送EOF，并且需要得到对方的回应时，调用close后就会有问题，这时就接收不到回复消息了。



（3）什么是半关闭？针对输出流执行半关闭的主机处于何种状态？半关闭会导致对方主机接收什么信息？

> 答：半关闭是指只关闭输入流或输出流。
>
> 只能接收数据，不能发送数据。
>
> 接收到EOF
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

#### 源代码

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

void error_handling(char *message);

int main(int argc, char* argv[]){
    struct hostent *host;
    struct sockaddr_in addr;

    if(argc!=2){
        printf("Usage: %s <IP>\n",argv[0]);
        exit(1);
    }

    memset(&addr, 0 , sizeof(addr));
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    host = gethostbyaddr((char *)&addr.sin_addr.s_addr, 4 , AF_INET);
    
    if(!host){
        error_handling("gethostbyaddr() error");
    }

    printf("Offical name: %s\n",host->h_name);
    for(int i=0; host->h_aliases[i]; ++i){
        printf("Offical name: %d: %s \n", i+1, host->h_aliases[i]);
    }

    printf("Address type : %s\n",(host->h_addrtype==AF_INET)?"AF_INET":"AF_INET6");

    for(int i=0; host->h_addr_list[i]; ++i){
        printf("IP addr %d : %s \n",i+1, inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
        // printf("IP addr %d : %s \n", i+1, inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
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
$ gcc gethostbyaddr.c -o gethostbyaddr
$ ./gethostbyaddr 202.108.22.5
Offical name: xd-22-5-a8.bta.net.cn
Address type : AF_INET
IP addr 1 : 202.108.22.5 
```



3、习题(参考答案)

（1）下列关于DNS的说法错误的是？

1. 因为DNS存在，故可以用域名替代IP
2. DNS服务器实际上是路由器，因为路由器根据域名决定数据的路径
3. 所有域名信息并非集中与 1 台 DNS 服务器，但可以获取某一 DNS 服务器中未注册的IP地址
4. DNS 服务器根据操作系统进行区分，Windows 下的 DNS 服务器和 Linux 下的 DNS 服务器是不同的。

> 答：a，b，c，d
>
> a：域名可以方便人类识别和记忆，原理上无法替代IP地址
>
> b：DNS服务器并不是路由器，且路由器是根据IP地址决定数据路径。
>
> c：只有注册了域名，并与IP地址绑定后，才可以从DNS服务器中获取
>
> d：DNS服务器在windows中和Linux下功能一致

![习题8.4.2](/Users/wangjun/computer-system/计算机网络原理/TCP-IP-网络编程/ch08/习题8.4.2.png)

> 答：可行，DNS服务器是分布式的，一台坏了，可以找其他的。

（3）再浏览器地址输入 www.orentec.co.kr ，并整理出主页显示过程。假设浏览器访问默认 DNS 服务器中并没有关于 www.orentec.co.kr 的地址信息.

> 答：主机先查询默认DNS服务器，不存在就继续查询更高级别的DNS服务器，若查询到，将对应的IP地址原路返回给发出请求的主机，然后该主机用户该IP请求对应的服务器的网页。

# 第9章 套接字的多种可选项

## 1、套接字可选项和I/O缓冲大小

### 1.1 套接字的可选项

通过套接字的可选项，可以控制更多套接字的底层工作信息

![套接字可选类型](/Users/wangjun/computer-system/计算机网络原理/TCP-IP-网络编程/ch09/套接字可选类型.png)

![套接字可选类型2](/Users/wangjun/computer-system/计算机网络原理/TCP-IP-网络编程/ch09/套接字可选类型2.png)



### 1.2 getsockopt()

该函数用于获取套接字的可选项信息

```c
#include <sys/socket.h>

int getsockopt(int sock, int level, int optname, void *optval, socklen_t *optlen);

//成功时返回0，失败时返回-1
/*
	sock 套接字文件描述符
	level 要查看的可选项的协议层
	optname 要查看的可选项名
	optval 保存查看结果的缓冲地址值
	optlen 向第4个参数optval传递的缓冲大小
*/
```

#### 实例：获取SO_TYPR

查看TCP套接字和UDP套接字的SO_TYPE信息

```c
int sock_type;
socklen_t optlen;
optlen = sizeof(sock_type);

getsockopt(tcp_sock, SOL_SOCKET, SO_TYPE, (void*)&sock_type, &optlen);
getsockopt(udp_sock, SOL_SOCKET, SO_TYPE, (void*)&sock_type, &optlen);
//tcp_sock  tcp套接字
```

SO_TYPE是典型的只读可选项，只能在创建时决定，以后不能更改。



### 1.3 setsockopt()

该函数用于设置套接字的可选项信息

```c
#include <sys/socket.h>

int setsockopt(int sock, int level, int optname, void *optval, socklen_t *optlen);
// 成功时返回0 失败时返回-1
/*
	sock 套接字文件描述符
	level 要查看的可选项的协议层
	optname 要查看的可选项名
	optval 保存查看结果的缓冲地址值
	optlen 向第4个参数optval传递的缓冲大小
*/
```



### 1.4 SO_SNDBUF & SO_RCVBUF

SO_SNDBUF表示输出缓冲区大小可选项

SO_RCVBUF表示输入缓冲区大小可选项



#### 实例：获取socket缓冲区大小

```c
int snd_buf, rcv_buf;
socklen_t optlen;

len = sizeof(snd_buf);
getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (void*)&snd_buf, &len);
len = sizeof(rcv_buf);
getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void*)&rcv_buf, &len);
```

结果可能因系统而异。



#### 实例：更改socket缓冲区大小

```c
int snd_buf = 1024*3, rcv_buf=1024*3;
socklen_t optlen;
optlen = sizeof(sock_type);

setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (void*)&snd_buf, sizeof(snd_buf));
setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void*)&rcv_buf, sizeof(rcv_buf));
```



## 2、SO_REUSEADDR

### 2.1 发生Binding Error

参照之前的回声客户端，当我们在客户端通过CTRL-C终止程序时，我们还可以重新连接服务器，似乎没有任何影响。

但当我们在服务器端通过CTRL-C终止程序时，再启动服务器端，就会发生Binding Error错误。为什么？因此此时端口号还未释放，因而无法再分配相同的端口号，但过了几分钟后，发现这个端口号又可以用了，至于为什么？先了解Time-wait状态



### 2.2 Time-wait状态

当服务器或客户端中的任何一方通过CTRL-C强制退出，或系统执行close函数退出程序，那么该套接字会进入 Time-wait状态，如下图所示。

<img src="/Users/wangjun/computer-system/计算机网络原理/TCP-IP-网络编程/ch09/Time-wait状态下的套接字.png" alt="Time-wait状态下的套接字" style="zoom:33%;" />



#### Time-wait出现原因

当套接字断开连接时，不管是CTRL-C(由操作系统关闭文件及套接字)或正常退出，会经过四次挥手，如图9-1，当主机A向主机B发送最后一次ACK数据包时立即断开连接，若之后该数据包丢失，主机B会以为之前传递的FIN消息(SEQ 7501，ACK 5001)未能抵达主机A，继而试图重传。但主机A已是完全终止的状态，因此主机B永远也无法收到主机A组后传来的ACK消息。相反，如果主机A的套接字处在Time-wait状态，则会向主机B重传最后的ACK消息，主机B也可以正常终止。

<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-04-30 上午11.23.06.png" alt="截屏2021-04-30 上午11.23.06" style="zoom: 33%;" />



客户端套接字端口号因为是随机分配。所以当再次连接服务器时，无法感受到Time-wait的存在。

当服务器端再次分配相同的端口号时，就会出现Binding Error错误，因为此时该端口号处于Time-wait状态，大概持续几分钟时间。





### 2.3 地址再分配

Time-wait看似重要，但不一定讨人系统，因为当系统出现故障，需要紧急重启时，但因处于Time-wait状态而必须等待几分钟，如果网络状态不理想，Time-wait状态还将持续。

解决方案就是在套接字可选项中更改SO_REUSEADDR的状态，适当调整该参数，可将Time-wait状态下的套接字端口号重新分配给新的套接字。SO_REUSEADDR的默认值为0，改为1就可以让将端口号分配给新的套接字。

```c
socklen_t optlen;
int option;
optlen = sizeof(option);
option = TURE;
setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&option, optlen);
```



## 3、TCP_NODELAY

### 3.1 Nagle 算法

为防止因数据包过多而发生网络过载，Nagle算法在1984年诞生，它应用于TCP层，其使用与否会导致下列差异：

<img src="/Users/wangjun/computer-system/计算机网络原理/TCP-IP-网络编程/ch09/Nagle算法.png" alt="Nagle算法" style="zoom:50%;" />

Nagle 算法具体内容：只有收到前一数据的ACK消息时，才发送下一数据。



Nagle算法并不是什么时候都适用。根据传输数据的特性，网络流量未受太大影响时，不使用Nagle算法要比使用它时传输速度更快。最典型的就是"传输大文件"时。将文件数据传输输出缓冲不会花太多时间，因此，即便不使用Nagle算法，也会在装满输出缓冲时传输数据包。这不仅不会增加数据包的数量，反而会在无需等待ACK的前提下连续传输，因而可以大大提高传输速度。



### 3.2 禁用Nagle算法

只需将TCP_NODELAY改为1

```c
int opt_val = 1;
setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void*)&opt_val, sizeof(opt_val));
```



也可以查看TCP_NODELAY值的设置状态

```c
int opt_val;
socklen_t opt_len;
opt_len = sizeof(opt_val);
getsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void*)&opt_val, &optlen);
```

opt_val为0表示正在使用使用Nagle算法，为1，表示已经禁用Nagle算法



## 4、习题（参考答案）

4.1 下列关于Time-wait状态的说法错误的是？

1. Time-wait 状态只在服务器的套接字中发生
2. 断开连接的四次握手过程中，先传输 FIN 消息的套接字将进入 Time-wait 状态。
3. Time-wait 状态与断开连接的过程无关，而与请求连接过程中 SYN 消息的传输顺序有关
4. Time-wait 状态通常并非必要，应尽可能通过更改套接字可选项来防止其发生

> 答：1，3，4



4.2 TCP_NODELAY 可选项与 Nagle 算法有关，可通过它禁用 Nagle 算法。请问何时应考虑禁用 Nagle 算法？结合收发数据的特性给出说明。

> 答：当传输大文件时，若网络流量未受太大影响时，可以禁用 Nagle 算法。因为这样可以在没有收到ACK消息的情况下连续发送数据包，可以大大提高传输速度。



# 第10章 多进程服务器端

## 1、进程的概念及应用

### 1.1 两种类型的服务器端

单进程下，每次只能受理一个客户端的请求，第1位连接请求的受理时间是0秒，第50位连接请求的受理时间是50秒，第100位连接请求的受理时间是100秒。很显然这种方式是不合理的，没有多少人有耐心为了一个页面请求等待几分钟。



多进程下，为每个请求新建一个进程，在宏观上相当于同时受理所有客户端的连接请求，每个客户端连接请求的受理时间为2～3秒，这种技术更加合理一些。



### 1.2 并发服务器端的实现方法

网络程序中数据通信时间比CPU运行时间占比更大，因此，向多个客户端提供服务是一种有效利用CPU的方式。下面是具有代表性的并发服务器端实现模型和方法：

* 多进程服务器：通过创建多个进程提供服务。
* 多路复用服务器：通过捆绑并统一管理I/O对象提供服务。
* 多线程服务器：通过生成与客户端等量的线程提供服务。

本章重点介绍多进程服务器。

### 1.3 理解进程

#### 进程的概念

进程（Process）是计算机中的程序关于某数据集合上的一次运行活动，是系统进行资源分配和调度的基本单位，是操作系统结构的基础。

简单说："进程就是占用空间的正在运行的程序!"

一个可执行程序，在开始运行前只是一个普通程序，即存放在磁盘中，包含二进制代码的文件。当运行该程序后，系统将该程序调入内存，就变为一个进程(包含二进制代码，及所使用的各种计算机资源)，该进程可以申请或释放计算机资源。

#### 单核和多核

单核cpu一次只能运行一个进程，当现代操作系统都采用分时使用cpu资源，即每个进程轮流使用cpu，只不过每个进程执行时间很短，所以造成了进程同时执行的错觉。

#### 进程查询

多核cpu可以同时执行多个进程，但进程数量往往大于核心数，所以进程往往都需要分时使用CPU资源。

```shell
$ ps au
USER       PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
gdm       1450  0.0  0.2 212136  6056 tty1     Ssl+ Apr29   0:00 /usr/lib/gdm3/gdm-x-sessio
root      1452  0.0  0.7 275792 16044 tty1     Sl+  Apr29   0:28 /usr/lib/xorg/Xorg vt1 -di
gdm       1514  0.0  0.6 558988 13992 tty1     Sl+  Apr29   0:00 /usr/lib/gnome-session/gno
gdm       1532  0.0  6.5 3101252 133240 tty1   Sl+  Apr29   1:45 /usr/bin/gnome-shell
gdm       1555  0.0  0.3 435100  7836 tty1     Sl   Apr29   0:00 ibus-daemon --xim --panel 
gdm       1558  0.0  0.2 280748  5912 tty1     Sl   Apr29   0:00 /usr/lib/ibus/ibus-dconf
gdm       1562  0.0  1.0 344012 20832 tty1     Sl   Apr29   0:00 /usr/lib/ibus/ibus-x11 --k
gdm       1589  0.0  1.0 494596 21748 tty1     Sl+  Apr29   0:00 /usr/lib/gnome-settings-da
gdm       1594  0.0  0.2 278160  5788 tty1     Sl+  Apr29   0:00 /usr/lib/gnome-settings-da
gdm       1596  0.0  0.9 343612 20112 tty1     Sl+  Apr29   0:00 /usr/lib/gnome-settings-da
```

使用ps命令可以查看当前系统正在运行的进程，每个进程都具有唯一的编号(PID)，一个系统能使用的PID数量是有限的，因而一个系统能使用的进程是有限的。



### 1.4 进程创建方法

创建进程的方法有很多，此处介绍用fork函数创建子进程。

```shell
#include <unistd.h>

pid_t fork(void);
//成功时返回进程IP，失败时返回-1
```

fork函数将创建调用的进程副本。也就是说：并非完全根据不同的程序创建进程，而是复制正在运行的，调用fork函数的进程。另外，两个函数都将执行fork函数调用后的语句（准确地说是在fork函数返回之后）。但因为通过同一个进程，复制相同的内存空间，之后的程序流要根据fork函数的返回值加以区分。即利用fork函数的如下特点区分程序的执行流程。

* 父进程：fork函数返回子进程ID
* 子进程：fork函数返回0

此处“父进程”指原进程，即调用fork函数的主体，二“子进程是通过父进程调用fork函数”复制出的进程。下图表示fork函数后的程序运行流程。

<img src="/Users/wangjun/computer-system/计算机网络原理/TCP-IP-网络编程/ch10/fork函数的调用.png" alt="fork函数的调用" style="zoom: 33%;" />



子进程和父进程都拥有执行fork函数之前的变量，但它们的运行不会相互影响。

下面给出例子验证之前的猜想：

#### 代码

```c
#include <stdio.h>
#include <unistd.h>

int gval = 10;
int main(int argc, char* argv[]){
    __pid_t pid;      //原书为pid_t，pid_t与__pid_t是的等价的
    int lval = 20;
    gval++, lval+=5;
    
    pid = fork();
    if(pid==0){
        gval==2, lval+=2;
    }else{
        gval-=2,lval-=2;
    }

    if(pid==0)
        printf("Child Proc: [%d, %d]\n",gval, lval);
    else
        printf("Parent Proc: [%d, %d]\n",gval, lval);

    return 0;
}
```



#### 编译运行

```shell
$ gcc fork.c -o ./bin/fork
$ ./bin/fork
Parent Proc: [9, 23]
Child Proc: [11, 27]
```



## 2、僵尸进程

### 2.1 概念

文件操作中关闭文件和打开文件同等重要。同样，进程销毁也和进程创建同等重要。如果未认真对待进程销毁，它们就会变成僵尸进程。

僵尸进程是当子进程比父进程先结束，而父进程又没有回收子进程，释放子进程占用的资源，此时子进程将成为一个僵尸进程。如果父进程先退出 ，子进程被init接管，子进程退出后init会回收其占用的相关资源。

### 2.2 产生原因

调用fork函数产生子进程的终止方式：

* 传递参数并调用exit()函数
* main函数中执行return语句并返回值

向exit函数传递的参数值和main函数的return语句返回的值都会传递给操作系统，而操作系统不会销毁子进程，直到把这些值传递给产生该子进程的父进程。处在这种状态下的进程就是**僵尸进程**。也就是说，是操作系统将子进程变成僵尸进程。那么，此进程何时被销毁呢？

**应该向创建子进程的父进程传递子进程的exit参数或return语句的返回值**

当然还没完，操作系统不会把这些值主动传递给父进程，只有父进程主动发起请求(函数调用)时，操作系统才会传递该值。

下面例子演示僵尸进程的产生：

#### 源码

```c
#include <stdio.h>
#include <unistd.h>

int gval = 10;
int main(int argc, char* argv[]){
    __pid_t pid;
    int lval = 20;
    gval++, lval+=5;
    
    pid = fork();
    if(pid==0){
        gval==2, lval+=2;
    }else{
        gval-=2,lval-=2;
    }

    if(pid==0)
        printf("Child Proc: [%d, %d]\n",gval, lval);
    else
        printf("Parent Proc: [%d, %d]\n",gval, lval);

    return 0;
}
```



#### 编译运行

```shell
$ gcc zombie.c -o ./bin/zombie
$ ./bin/zombie
Child Process ID : 5399 
Hi, I am a child process
End child process
END parent process
```

```shell
# 在另一个终端窗口中执行
$ ps au
mygit     5482  0.0  0.2  29808  5032 pts/3    Ss   12:39   0:00 /bin/bash
mygit     5532  0.0  0.0   4512   824 pts/2    S+   12:40   0:00 ./bin/zombie
mygit     5533  0.0  0.0      0     0 pts/2    Z+   12:40   0:00 [zombie] <defunct>
```

当父进程处于睡眠状态时，子进程为僵尸态。



上面的结果需要用另一个终端窗口来验证进程的状态，也可以将zombie后台运行，这样就只需要一个窗口就可以完成验证。`$ ./bin/zombie & `即可在后台运行



## 3 销毁僵尸进程

### 3.1 利用wait函数

```c
#include <sys/wait.h>

pid_t wait(int *statloc);
//成功时返回终止的子进程ID，失败时返回-1
```



调用次函数时如果已有子进程终止，那么子进程终止时传递的返回值(exit函数的参数值，main函数的return返回值)将保存到该函数的参数所指内存空间。但函数参数指向的单元中还包含其他信息，因此需要通过下列宏进行分离。

* WIFEXITED子进程正常终止时返回true
* WEXITSTATUS返回子进程的返回值

也就是，向wait函数传递变量status的地址时，调用wait函数后应该编写如下代码。

```
if(WIFEXITED(status)){
	puts("Normal termination!");
	printf("Child pass num: %d",WEXITSTATUS(status));
}
```

**调用wait函数时若没有子进程终止，那么程序将阻塞，直到有子进程终止，因此需谨慎调用该函数**

#### 演示代码

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc,char* argv[]){
    int status;
    pid_t pid = fork();

    if(pid==0){
        return 3;
    }else{
        printf("Child PID: %d \n",pid);
        pid = fork();
        if(pid==0){
            exit(7);
        }else{
            printf("Child PID: %d \n",pid);
            wait(&status);
            if(WIFEXITED(status))
                printf("Child send one: %d \n",WEXITSTATUS(status));

            wait(&status);
            if (WIFEXITED(status))
                printf("Child send one: %d \n", WEXITSTATUS(status));

            sleep(30);
        }
    }
}
```



#### 编译运行

```shell
$ gcc wait.c  -o ./bin/wait
$ ./bin/wait 
Child PID: 19403 
Child PID: 19404 
Child send one: 3 
Child send one: 7 
```

```shell
# 在另一个终端窗口中执行
$ ps au
mygit    10870  0.0  0.2  29940  5260 pts/0    Ss   23:11   0:00 /bin/bash
root     13828  0.0  0.1  72248  3836 pts/1    S    14:03   0:00 su - root
root     13835  0.0  0.2  29952  5372 pts/1    S+   14:03   0:00 -su
mygit    19756  0.0  0.0   4512   752 pts/0    S+   23:37   0:00 ./bin/wait
mygit    19787  0.0  0.1  46776  3620 pts/4    R+   23:37   0:00 ps au

# 可以发现并未产生僵尸进程
```



### 3.2 利用waitpid函数

wait函数会引起程序阻塞，还可以考虑使用waitpid函数。

```c
#include <sys/wait.h>

pid_t waitpid(pid_t pid, int *statloc, int options);
//成功时返回终止的子进程ID(或0)，失败时返回-1

/*
	pid 等待终止的目标子进程的ID，若传递-1， 则与wait函数相同，可以等待任意子进程终止
	statloc 与wait函数的statloc具有相同的含义
	options 传递头文件sys/wait.h中声明的常量WNOHANG，即使没有终止的子进程也不会进入阻塞状态，而是返回0并退出函数
*/
```

#### 演示代码

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[]){
    int status;
    pid_t pid = fork();
    if(pid==0){
        sleep(15);
        return 24;
    }else{
        while(!waitpid(-1, &status,WNOHANG)){
            sleep(3);
            puts("sleep 3sec.");
        }

        if(WIFEXITED(status)){
            printf("Child send %d \n",WEXITSTATUS(status));
        }
    }
    return 0;
}c
```

#### 编译运行

```shell
$ gcc waitpid.c -o ./bin/waitpid
$ ./bin/waitpid 
sleep 3sec.
sleep 3sec.
sleep 3sec.
sleep 3sec.
sleep 3sec.
Child send 24 
```



## 4、信号处理

“子进程究竟何时终止？调用waitpid函数后要无休止地等待吗？”

父进程往往与子进程一样繁忙，因此不能只调用waitpid函数以等待子进程终止。

### 4.1 向操作系统求助

子进程终止的识别主体是操作系统，因此，若操作系统在子进程结束时，能给它的父进程发一个消息，"嘿，父进程，你创建的子进程终止了！"，此时父进程将暂时放下工作，处理子进程终止的相关事宜。这应该更加合理。为了实现该想法，我们引入了信号处理(Signal Handling)机制。

此处“信号”是指在特定事件发生时由操作系统向进程发送的消息。

另外，为了响应消息，执行与消息相关的自定义操作的过程称为“信号处理”。



### 4.2 信号与signal函数

为了销毁终止的进程，需要父进程提前设定好一个"信号处理函数"，当进程终止时，由操作系统执行该函数。

```c
#include <signal.h>

void (*signal(int signo, void (*func)(int)))(int);
// 为了在产生信号时调用，返回之前注册的函数指针
/*
	函数名：signal
	参数：int signo, void (*func)(int)
	返回类型：参数为int型，返回void型指针
*/
```

signo的可选值：

SIGALRM：已到通过调用alarm函数注册的时间。

SIGINT：输入CTRL+C

SIGCHLD：子进程终止



接下来编写signal函数的调用语句，分别完成以下两个请求：

1. 已到通过alarm函数注册的时间，请调用timeout函数
2. 输入CTRL+C时调用keycontrol函数

很简单，编写完相应的函数，调用signal函数即可

signal(SIGALRM, timeout);

signal(SIGINT, keycontrol);



alerm函数声明

```c
#include <unistd.h>

unsigned int alarm(unsigned int seconds);
// 返回0或以秒为单位的距SIGALRM信号发生所剩时间
// 若传递0，则之前对SIGALRM信号的预约取消#include <unistd.h>
```

#### 演示代码

signal.c

```c
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void timeout(int sig){
    if(sig == SIGALRM){
        puts("Time out!");
    }

    alarm(2);
}

void keycontrol(int sig){
    if(sig == SIGINT){
        puts("CTRL+C pressed");
    }
}

int main(int argc, char* argv){
    signal(SIGALRM,timeout);
    signal(SIGINT, keycontrol);

    alarm(2);
    for(int i=0; i<3; i++){
        puts("wait...");
        sleep(100);
    }
    return 0;
}
```

#### 编译运行

```shell
$ gcc signal.c -o ./bin/signal
$ ./bin/signal 
wait...
Time out!
wait...
Time out!
wait...
Time out!
```

puts("wait...")执行之后，并没有休眠100秒，而是在2秒后就输出了Time out!，因为**发生信号时将唤醒由于调用sleep函数而进入阻塞状态的进程**



### 4.3 利用sigaction函数进行信号处理

signal函数与sigaction函数的区别：signal函数在UNIX系列的不同操作系统中可能存在区别，但sigaction函数完全相同。

实际开发中很少用signal函数编写程序，只是为了与旧版程序的兼容。为了更好的稳定性，应该使用sigaction函数进行信号处理。

sigaction函数声明

```c
#include <signal.h>

int sigaction(int signo, const struct sigaction *act, struct sigaction *oldact);
//成功时返回0，失败时返回-1

/*
	signo 与signal函数相同，传递信号信息
	act 信号处理函数
	oldact 获取之前注册的信号处理函数指针，若不需要则传递0
*/
```

声明并初始化sigaction结构体变量以调用上述函数

```c
struct sigaction{
	void (*sa_handler)(int);     //信号处理函数
  sigset_t sa_mask;						 
  int sa_flags;
}
```

sigaction.c

#### 演示代码

```c
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void timeout(int sig)
{
    if (sig == SIGALRM)
    {
        puts("Time out!");
    }

    alarm(2);
}

void keycontrol(int sig)
{
    if (sig == SIGINT)
    {
        puts("CTRL+C pressed");
    }
}

int main(int argc, char *argv)
{
    struct sigaction act;
    act.sa_handler = timeout;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGALRM, &act, 0);
    alarm(2);
    for (int i = 0; i < 3; i++)
    {
        puts("wait...");
        sleep(100);
    }
    return 0;
}
```

#### 编译运行

```shell
$ gcc sigaction.c -o ./bin/sigaction
$ ./bin/sigaction 
wait...
Time out!
wait...
Time out!
wait...
Time out!
```



## 5、利用信号处理技术消灭僵尸进程

子进程终止时将产生SIGCHLD信号，接下来利用sigaction函数编写实例：

#### 代码示例 

remove_zombie.c

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

void read_childproc(int sig){
    int status;
    pid_t id = waitpid(-1, &status, WNOHANG); //WNOHANG为
    if(WIFEXITED(status)){
        printf("Remove oroc id: %d\n", id);
        printf("Child send: %d \n",WEXITSTATUS(status));
    }
}


int main(int argc, char *argv[])
{
    pid_t pid;
    struct sigaction act;
    
    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, 0);
    pid = fork();
    if(pid==0){
        puts("Hi，I'm child process");
        sleep(10);
        return 12;
    }else{
        printf("Child proc id : %d\n",pid);
        pid = fork();
        if(pid==0){
            puts("Hi, I'm child process");
            sleep(10);
            exit(24);
        }else{
            printf("Child proc id : %d\n",pid);
            for(int i=0; i<5; i++){
                puts("wait...");
                sleep(5);
            }
        }
    }
    return 0;
}
```

#### 编译运行

```shell
$ gcc remove_zombie.c -o ./bin/remove_zombie 
$ ./bin/remove_zombie 
Child proc id : 8657
Child proc id : 8658
wait...
Hi, I'm child process
Hi，I'm child process
wait...
Remove oroc id: 8658
Child send: 24 
wait...
Remove oroc id: 8657
Child send: 12 
wait...
wait...
```

当子进程结束时产生SIGCHLD信号，由操作系统代执行read_childproc函数来销毁子进程。



## 6、基于多任务的并发服务器

### 6.1 基于进程的并发服务器模型

为每一个请求连接的客户端提供一个进程提供服务，这样就可以同时为多个客户端提供服务。实现分三个阶段：

1. 回声服务器端（父进程）通过调用accept函数受理连接请求。
2. 此时获取的套接字文件描述符创建并传递给子进程
3. 子进程利用传递来的文件描述符提供服务

#### 示例代码

echo_mpserv.c

```c
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>

#define BUF_SIZE 1024
void errorHandling(const char *message);
void read_childproc(int sig);

int main(int argc, char *argv[])
{
    int str_len,state;
    int serv_sock, clnt_sock;
    char message[BUF_SIZE];

    pid_t pid;
    struct sigaction act;
    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    state = sigaction(SIGCHLD, &act, 0);


    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    if (argc != 2)
    {
        printf("Usage: %s <port> \n", argv[1]);
        exit(1);
    }

    //调用socket函数创建套接字
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        errorHandling("socket() error");
    }
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    //调用bind函数分配IP地址和端口号
    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("bind() error!");
    }

    //调用listen函数将套接字转化为可接收连接状态
    if (listen(serv_sock, 5) == -1)
    {
        errorHandling("listen() error!");
    }
    clnt_adr_sz = sizeof(clnt_adr);
    //write函数用于传输数据，若程序经accept函数运行到本行，说明已经有了连接请求
    //调用accept函数受理连接请求，如果在没有连接请求的情况下调用该函数，则不会返回，直到有连接请求为止。

    while(1){

        //调用accept函数受理连接请求，如果在没有连接请求的情况下调用该函数，则不会返回，直到有连接请求为止。
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
        if (clnt_sock == -1)
        {
            continue;
        }
        else
        {
            printf("new connected client\n");
        }
        pid = fork();
        if(pid == -1){
            close(clnt_sock);
            continue;
        }
        if(pid==0){
            close(serv_sock);
            while ((str_len = read(clnt_sock, message, BUF_SIZE))){
                //str_len表示读取到的字符串长度
                write(clnt_sock, message, str_len);
            }

            close(clnt_sock);
            puts("client disconnected...");
            return 0;
        }else{
            close(clnt_sock);
        }
    }

    close(serv_sock);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void read_childproc(int sig){
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG); //-1代表可以等待任意子进程终止  WNOHANG即使没有终止的子进程也不会进入阻塞状态，而是返回0并退出函数
    printf("remove proc id: %d\n",pid);
}
```



#### 编译运行

```shell
$ gcc echo_mpserv.c -o ./bin/echo_mpserv
$ ./bin/echo_mpserv 9190
new connected client
client disconnected...
remove proc id: 20943
```

```shell
$ gcc echo_mpclient.c -o ./bin/echo_mpclient
$ ./bin/echo_mpclient 127.0.0.1 9190
Connected......
Input message(Q to quit):hello world
Message from server: hello world
Input message(Q to quit):how are you
Message from server: how are you
Input message(Q to quit):123
Message from server: 123
Input message(Q to quit):q
```



## 7 分割TCP的I/O程序

在上面的示例中，对于客户端：向服务器端传输数据，并等待服务器的回复。无条件等待，直到接受完服务器端的回声数据后，才能传输下一批数据。



### 7.1分割I/O程序的优点

即读写分离，分别用独立的两个进程来读取数据和发送数据。这样可以提高数据传输的效率。

* 在1个进程内同时实现数据收发逻辑需要考虑更多细节。程序越复杂，这种区别越明显，这也是公认的优点。
* 可以提高频繁交换数据的程序性能

#### 示例代码

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void errorHandling(const char *message);
void read_routine(int sock, char *buf);
void write_routine(int sock, char *buf);
int itoc(int num, char *str);
int main(int argc, char *argv[])
{
    int sock;
    char message[BUF_SIZE];
    int str_len;
    struct sockaddr_in serv_adr;

    pid_t pid;


    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        errorHandling("socket() error");
    }

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("connect() error!");
    }
    else
    {
        puts("Connected......");
    }

    pid = fork();
    if(pid==0){
        write_routine(sock,message);
    }else{
        read_routine(sock, message);
    }
    close(sock);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void read_routine(int sock, char *buf){
    while(1){
        int str_len = read(sock, buf, BUF_SIZE);
        if(str_len == 0)
            return;

        buf[str_len] = 0;
        printf("Message from server: %s",buf);
    }
}

void write_routine(int sock, char *buf){
    while(1){
        fgets(buf,BUF_SIZE, stdin);
        if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n")){
            shutdown(sock, SHUT_WR);
            return;
        }

        write(sock,buf,strlen(buf));
        
    }
}

int itoc(int num, char *str)
{
    char tem[1024];
    int id = 0, id2 = 0;
    while (num)
    {
        int t = num % 10;
        tem[id++] = t + '0';
        num /= 10;
    }
    str[id--] = '\0';
    while (id >= 0)
    {
        str[id2++] = tem[id--];
    }
    return 0;
}
```



#### 编译运行

```shell
$ gcc echo_mpclient.c -o ./bin/echo_mpclient 
$ ./bin/echo_mpclient 127.0.0.1 9190
Connected......
hello world
Message from server: hello world
hhh 
Message from server: hhh
aaa
Message from server: aaa
bbb
Message from server: bbb
q
```



## 8、习题（参考答案）

(1) 下列关于进程的说法错误的是：

a. 从操作系统的角度上说，进程是程序运行的单位
b. 进程根据创建方式建立父子关系
c. 进程可以包含其他进程，即一个进程的内存空间可以包含其他进程
d. 子进程可以创建其他子进程，而创建出来的子进程还可以创建其他子进程，但所有这些进程只与一个父进程建立父子关系。

> 答：
>
> c：每进程的都含有独立的内存空间
>
> d：子进程a创建的子进程b，a与b是父子关系，但a的父进程与b不是父子关系。



(2) 调用fork函数创建子进程，以下关于子进程的描述错误的是？

a. 父进程销毁时也会同时销毁子进程
b. 子进程是复制父进程所有资源创建出的进程
c. 父子进程共享全局变量
d. 通过 fork 函数创建的子进程将执行从开始到 fork 函数调用为止的代码。

> 答：
>
> c：父子进程分别含有全局变量
>
> d：子进程具有父进程的所有资源，包括所有代码，除了代码块if(pid==0){}else{}中else部分



(3) 创建子进程时将复制父进程的所有内容，此时的复制对象也包含套接字文件描述符。编写程序验证复制的文件描述符整数值是否与原文件描述符整数值相同。

```c
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char* argv[]){

    int sock;
    pid_t pid;
    sock = socket(PF_INET, SOCK_STREAM, 0);
    pid = fork();
    if(pid==0){
        printf("Child proc sock %d\n",sock);
    }else{
        printf("Parent proc sock %d\n",sock);
    }
    return 0;
}
```

编译运行

```shell
$ gcc test-sock.c -o ./bin/test-sock
$ ./bin/test-sock 
Parent proc sock 3
Child proc sock 3
```



(4)请说明进程变为僵尸进程的过程及预防措施？

进程变为僵尸进程往往是子进程已经执行结束，父进程没有及时销毁子进程，此时子进程就变成僵尸进程。

预防措施：父进程主动销毁子进程，可以通过wait函数，waitpid函数，还可以通过信号处理技术，委托操作系统在适当时候销毁子进程。



![截屏2021-05-07 上午12.53.42](https://cdn.jsdelivr.net/gh/wangjunstf/pics@main/uPic/%E6%88%AA%E5%B1%8F2021-05-07%20%E4%B8%8A%E5%8D%8812.53.42.png)

```c
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>


void keycontrol(int sig){
    char msg[10];
    if(sig==SIGINT){
        puts("退出请输入y");
        fgets(msg, sizeof(msg), stdin);
        if (!strcmp("y\n", msg) || !strcmp("Y\n", msg))
        {
            exit(0);
        }
    }
}

int main(int argc, char* argv[]){
    signal(SIGINT,keycontrol);

    while(1){
        puts("Hello world!");
        sleep(1);
    }
    return 0;
}
```



编译运行

```shell
$ gcc print.c -o ./bin/print
$ ./bin/print
Hello world!
Hello world!
Hello world!
Hello world!
^C退出请输入y
y
```

# 第 11 章 进程间通信

因为不同的进程占用不同的内存空间，无法直接进行通行，只能借助其他特殊方法完成。

**本章介绍进程间通信的一种方式：管道**

## 1、进程间通信的方式

### 1.1 通过管道实现进程间通信

为了完成进程间通信，需要创建管道。管道并非属于进程的资源，而是和套接字一样，属于操作系统。管道就是一段内存空间，通过使用管道，两个进程通过操作系统提供的内存空间进行通信。

管道的创建

```c
#include <unistd.h>

int pipe(int filedes[2]);
//成功时返回0，失败时返回-1
/*
	filedes[0] 通过管道接收数据时使用的文件描述符，即管道出口。
	filedes[1] 通过管道传输数据时使用的文件描述符，即管道入口
*/
```

#### 示例

```c
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

#define BUF_SIZE 30

int main(int argc, char* argv[]){
    int fds[2];
    char str[] = "Who are you?";
    char buf[BUF_SIZE];
    pid_t pid;

    pipe(fds);
    pid = fork();
    if(pid == 0 ){
        write(fds[1],str, sizeof(str));
    }else{
        read(fds[0],buf,BUF_SIZE);
        puts(buf);
    }
    return 0;
}
```

#### 编译运行

```shell
$ gcc pipe1.c -o ./bin/pipe1
$ ./bin/pipe1
Who are you?
```

上述示例中，子进程往管道写数据，父进程往管道读取数据，如下图所示：

<img src="https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-07%20%E4%B8%8B%E5%8D%888.59.05.png" alt="截屏2021-05-07 下午8.59.05" style="zoom:33%;" />

### 1.2 通过管道进行进程间双向通信

下面创建2个进程通过1个管道进行双向数据交换，其通信方式如下图所示：

<img src="https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-07%20%E4%B8%8B%E5%8D%889.04.34.png" alt="截屏2021-05-07 下午9.04.34" style="zoom: 67%;" />

#### 代码示例

```c
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

#define BUF_SIZE 30

int main(int argc, char *argv[])
{
    int fds[2];
    char str1[] = "Who are you?";
    char str2[] = "Thank for your message";
    char buf[BUF_SIZE];
    pid_t pid;

    pipe(fds);
    pid = fork();
    if (pid == 0)
    {
        write(fds[1], str1, sizeof(str1));
        sleep(2);
        read(fds[0], buf, BUF_SIZE);
        printf("Child proc output: %s \n",buf);
    }
    else
    {
        read(fds[0], buf, BUF_SIZE);
        printf("Parent proc output: %s \n",buf);
        write(fds[1], str2, sizeof(str2));
        sleep(3);
    }
    return 0;
}
```



#### 编译运行

```shell
$ gcc pipe2.c -o ./bin/pipe2
$ ./bin/pipe2
Parent proc output: Who are you? 
Child proc output: Thank for your message 
```



子进程向管道写入输入后，休眠2秒，父进程从管道读取完数据，并向管道写入数据，然后又进入休眠。子进程休眠结束后从管道读取数据。

这里有个疑问，是子进程先写入，还是父进程先读取？

> 其实这个没有区别，如果是父进程先尝试读取，因为管道没有数据会进入阻塞状态，直到子进程向管道写入数据，子进程向管道写入了数据，就进入了休眠模式，父进程在子进程休眠过程中向管道写入数据，然后进入休眠，子进程在父进程休眠期间从管道读取数据。



从上面的示例中，可以看到只用1个管道进行双向通信并非易事。为了实现这一点，程序需要预测并控制运行流程，这在每种系统中都不同，可以视为不可能完成的任务。



既然如此，该如何完成双向通信？答案是“使用2个管道。”

### 1.3 使用2个管道完成双向通信

使用2个管道完成双向通信，如下图所示：

<img src="https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-07%20%E4%B8%8B%E5%8D%889.17.36.png" alt="截屏2021-05-07 下午9.17.36" style="zoom:50%;" />

#### 代码示例

```c
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

#define BUF_SIZE 30

int main(int argc, char *argv[])
{
    int fds1[2], fds2[2];
    char str1[] = "Who are you?";
    char str2[] = "Thank for your message";
    char buf[BUF_SIZE];
    pid_t pid;

    pipe(fds1);
    pipe(fds2);
    pid = fork();
    if (pid == 0)
    {
        write(fds1[1], str1, sizeof(str1));
        read(fds2[0], buf, BUF_SIZE);
        printf("Child proc output: %s \n", buf);
    }
    else
    {
        read(fds1[0], buf, BUF_SIZE);
        printf("Parent proc output: %s \n", buf);
        write(fds2[1], str2, sizeof(str2));
        //sleep(3);
    }
    return 0;
}
```



#### 编译运行

```shell
$ gcc pipe3.c -o ./bin/pipe3
$ ./bin/pipe3
Parent proc output: Who are you? 
Child proc output: Thank for your message 
```



## 2、运用进程间通信

接下来扩展第10章echo_mpserv.c，添加以下功能：将回声客户端传输的字符串按序保存到文件中。

我们将该任务委托给另外的进程。换言之，另行创建进程，从向客户端提供服务的进程读取字符串信息。当然，该过程需要创建用于接收数据的管道。

#### 代码示例

echo_storeserv.c

```c
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>

#define BUF_SIZE 1024
void errorHandling(const char *message);
void read_childproc(int sig);

int main(int argc, char *argv[])
{
    int str_len, state;
    int serv_sock, clnt_sock;
    char message[BUF_SIZE];
    int fds[2];

    pid_t pid;
    struct sigaction act;
    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    state = sigaction(SIGCHLD, &act, 0);

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    if (argc != 2)
    {
        printf("Usage: %s <port> \n", argv[1]);
        exit(1);
    }

    //调用socket函数创建套接字
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        errorHandling("socket() error");
    }
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    //调用bind函数分配IP地址和端口号
    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("bind() error!");
    }

    //调用listen函数将套接字转化为可接收连接状态
    if (listen(serv_sock, 5) == -1)
    {
        errorHandling("listen() error!");
    }
    //write函数用于传输数据，若程序经accept函数运行到本行，说明已经有了连接请求
    //调用accept函数受理连接请求，如果在没有连接请求的情况下调用该函数，则不会返回，直到有连接请求为止。

    pipe(fds);
    pid = fork();
    if(pid==0){
        FILE *fp = fopen("echomsg.txt","wt");
        char msgbuf[BUF_SIZE];
        int len;
        for(int i=0; i<5; i++){
            len = read(fds[0],msgbuf, BUF_SIZE);
            fwrite((void*)msgbuf, 1 , len, fp);
            
        }

        fclose(fp);
        return 0;
    }

    while (1)
    {
        clnt_adr_sz = sizeof(clnt_adr);
        //调用accept函数受理连接请求，如果在没有连接请求的情况下调用该函数，则不会返回，直到有连接请求为止。
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
        if (clnt_sock == -1)
        {
            continue;
        }
        else
        {
            printf("new connected client\n");
        }
        pid = fork();
        if (pid == -1)
        {
            close(clnt_sock);
            continue;
        }
        if (pid == 0)
        {
            close(serv_sock);
            while ((str_len = read(clnt_sock, message, BUF_SIZE)))
            {
                //str_len表示读取到的字符串长度
                write(clnt_sock, message, str_len);
                write(fds[1],message,str_len);
            }

            close(clnt_sock);
            puts("client disconnected...");
            return 0;
        }
        else
        {
            close(clnt_sock);
        }
    }

    close(serv_sock);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void read_childproc(int sig)
{
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG); //-1代表可以等待任意子进程终止  WNOHANG即使没有终止的子进程也不会进入阻塞状态，而是返回0并退出函数
    printf("remove proc id: %d\n", pid);
}

```

echo_mpclient.c

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void errorHandling(const char *message);
void read_routine(int sock, char *buf);
void write_routine(int sock, char *buf);
int itoc(int num, char *str);

int main(int argc, char *argv[])
{
    int sock;
    char message[BUF_SIZE];
    int str_len;
    struct sockaddr_in serv_adr;

    pid_t pid;

    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        errorHandling("socket() error");
    }

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("connect() error!");
    }
    else
    {
        puts("Connected......");
    }

    pid = fork();
    if (pid == 0)
    {
        write_routine(sock, message);
    }
    else
    {
        read_routine(sock, message);
    }
    close(sock);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void read_routine(int sock, char *buf)
{
    while (1)
    {
        int str_len = read(sock, buf, BUF_SIZE);
        if (str_len == 0)
            return;

        buf[str_len] = 0;
        printf("Message from server: %s", buf);
    }
}

void write_routine(int sock, char *buf)
{
    while (1)
    {
        fgets(buf, BUF_SIZE, stdin);
        if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n"))
        {
            shutdown(sock, SHUT_WR);
            return;
        }

        write(sock, buf, strlen(buf));
    }
}

int itoc(int num, char *str)
{
    char tem[1024];
    int id = 0, id2 = 0;
    while (num)
    {
        int t = num % 10;
        tem[id++] = t + '0';
        num /= 10;
    }
    str[id--] = '\0';
    while (id >= 0)
    {
        str[id2++] = tem[id--];
    }
    return 0;
}
```

#### 编译运行

服务器端

```shell
$ gcc echo_storeserv.c -o ./bin/echo_storeserv 
$ ./bin/echo_storeserv 9191
new connected client
remove proc id: 7616
client disconnected...
```

客户端

```shell
$ gcc echo_mpclient.c -o ./bin/echo_mpclient
$ ./bin/echo_mpclient 127.0.0.1 9191
Connected......
hello world
Message from server: hello world
how are you
Message from server: how are you
123        
Message from server: 123
code
Message from server: code
last message
Message from server: last message
q
```

服务器端接收完5条数据后，可以在echomsg.txt中验证保存的字符串。



## 3、习题(参考答案)

(1) 什么是进程间通信？分别从概念上和内存的角度进行说明？

> 答：进程间通信是指两个不同进程间可以交换数据。
>
> 从内存角度看，操作系统提供了两个进程可以同时访问的内存空间，从而可以让两个进程进行通信。



(2) 进程间通信需要特殊的IPC机制，这是由操作系统提供的。进程间通信 为何需要操作系统的帮助？

> 答：因为不同的进程具有完全独立的内存结构，无法直接进行通信。只能借助一块两个进程都可以访问的内存空间来进行通信。这块共享内存空间由操作系统提供。



(3) "管道"是典型的IPC技法。关于管道，请回答如下问题。

a. 管道是进程间交换数据的路径。如何创建此路径，由谁创建？

> 答：通过调用pipe函数创建，传递一个含有两个元素的数组，0号元素表示接收数据时使用的文件描述符，1号元素表示发送数据时使用的文件描述符。由操作系统创建。

b. 为了完成进程间通信，2个进程需同时连接管道。那2个进程如何连接到同一个管道？

> 答：调用fork函数，子进程会复制父进程的所有资源，包括文件描述符。因而父子进程都具有管道的I/O文件描述符。子进程和父进程都可以向同一个管道发送和接收数据，从而实现进程间通信。

c. 管道允许进行2个进程间的双向通信。双向通信中需要注意哪些内容？

> 注意读取和写入的次序，如果在写之前尝试读，那么调用read函数会进入阻塞状态，直到有数据进入管道。



(4) 编写示例复习IPC技法，使2个进程相互交换3次字符串。当然，这2个进程应该具有父子关系，各位可指定任意字符串。

```c
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#define BUF_SIZE 30

int main(int argc, char* argv[]){
    int fds[2];
    char buf[BUF_SIZE];
    char str1[] = "hello I am client.";
    char str2[] = "Nice to meet you.";
    char str3[] = "Nice to meet you too.";
    pid_t pid;
    int len;

    pipe(fds);
    pid = fork();
    if(pid==0){
        write(fds[1],str1,strlen(str1));
        sleep(2);
        len = read(fds[0], buf, BUF_SIZE);
        buf[len] = 0;
        printf("Child proc output: %s \n", buf);
        write(fds[1], str3, strlen(str3));
    }else{
        len = read(fds[0],buf,BUF_SIZE);
        buf[len] = 0;
        printf("Parent proc output: %s \n",buf);
        write(fds[1],str2,strlen(str2));
        sleep(3);
        len = read(fds[0], buf, BUF_SIZE);
        buf[len] = 0;
        printf("Parent proc output: %s \n", buf);
    }
    return 0;
}
```



**子进程**向管道**写入数据**，然后**睡眠2秒**，在这2秒内**父进程**从管道**读取数据**，并向管道**写入数据**，**父进程睡眠3秒**

**子进程**在父进程睡眠3秒的时间中读取父进程写入的数据，并写入数据。

父进程读取子进程写入的数据。



# 第 12 章 I/O复用

## 1、多进程服务器端的缺点和解决方法

为了构建并发服务器端，只要有客户端连接请求就会创建新进程。这的确是一种解决方法，但并非十全十美，因为创建进程需要付出极大代价，需要大量的运算和内存空间，由于每个进程都具有独立的内存空间，所以相互间的数据交换也要求采用相对复杂的方法（IPC属于相对复杂的通信方法）。



那有什么办法在不创建进程的同时向多个客户提供服务？答案是**I/O复用**。



## 2、理解I/O复用

“复用”在通信工程领域的含义是：”在1个通信频道中传递多个数据（信号）的技术“

更通俗一些，“复用”的含义是“为了提高物理设备的效率，用最少的物理要素传递最多数据时使用的技术”



服务器端引入复用技术，可以减少所需进程数。



I/O复用可以理解为用一个进程可以为多个客户提供服务。



## 3、理解select函数并实现服务器端

运用select函数是最具代表性的实现复用服务器端方法。

### 3.1 select函数的功能和调用顺序

使用select函数时可以将多个文件描述符集中到一起统一监视，监视的项目如下：

* 是否存在套接字接收数据？
* 无需阻塞传输数据的套接字有那些？
* 哪些套接字发生了异常？

select函数是整个I/O复用的全部内容，以下为select函数的调用顺序

<img src="https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-08%20%E4%B8%8B%E5%8D%889.35.55.png" alt="截屏2021-05-08 下午9.35.55" style="zoom: 67%;" />

### 3.2 设置文件描述符

利用select函数可以同时监视多个文件描述符。当然，监视文件描述符可以视为监视套接字。集中时也要按照监视项（接收，传输，异常）进行区分，共分成3类。

每一类都用fd_set结构体表示，如下图所示：

![截屏2021-05-08 下午11.02.40](https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-08%20%E4%B8%8B%E5%8D%8811.02.40.png)

上图是存有0和1的位数组，下标表示文件描述符，数组元素为1表示该文件描述符为监视对象。

要设置某个文件描述为监视对象（称为注册），或者取消对某个文件描述符的监视，需要更改每一位的值。在fd_set变量中注册或更改值的操作都由下列宏完成。

* FD_ZERO(fd_set *fdset)    : 将fd_set变量的所有位初始化为0
* FD_SET(int fd, fd_set *fdset)  : 在参数fd_set指向的变量中注册文件描述符fd的信息
* FD_CLR(int fd, fd_set *fdset)  : 从参数fdset指向的变量中清除文件描述符fd的信息
* FD_ISSET(int fd, fd_set *fdset) : 若参数fdset指向的变量中包含文件描述符fd的信息，则返回真



<img src="https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-08%20%E4%B8%8B%E5%8D%8811.12.06.png" alt="截屏2021-05-08 下午11.12.06" style="zoom: 67%;" />

 

### 3.3 设置监视范围及超时

```c
#include <sys/select.h>
#include <sys/time.h>

int select(int maxfd, fd_set *readset, fd_set *writeset, fd_set *exceptset, const struct timeval *timeout);
//成功时返回 发生事件的文件描述符数 失败时返回-1

/*
	maxfd 监视对象文件描述符数量
	readset 将所有关注"是否存在待读数据"的文件描述符注册到fd_set型变量，并传递其地址值
	writeset 将所有关注"是否可传输无阻塞数据"的文件描述符注册到fd_set型变量，并传递其地址值
	exceptset 将所有关注“是否发生异常”的文件描述符注册到fd_set型变量，并传递其地址值
	timeout 调用select函数后为防止陷入无限阻塞状态，传递超时(time-out)信息
*/
```



select函数只有在监视的文件描述符发生变化时才返回，如果未发生变化，就会进入阻塞状态。指定超时时间就是为了防止这种情况的发生。struct timeval的结构体内容

```c
struct timeval{
	long tv_sec;   //秒
	long tv_usec;  //毫秒
}
```

如果想设置超时，则传递NULL



### 3.4 调用select函数后查看结果

调用select函数后，可以知道哪些文件描述符发生了变化，发生变化的文件描述符被标记为1，没有发生变化的文件描述符被标记为0，如下图所示：

<img src="https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-08%20%E4%B8%8B%E5%8D%8811.27.36.png" alt="截屏2021-05-08 下午11.27.36" style="zoom: 67%;" />



每次调用select函数后，fd_set值会被修改，因此在调用select函数时，传递一个值与fd_set变量相等的临时变量。



### 3.5 select函数调用示例

#### 代码

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 30

int main(int argc, char* argv[]){
    fd_set reads,temps;
    int result, str_len;
    char buf[BUF_SIZE];
    struct timeval timeout;

    FD_ZERO(&reads);
    FD_SET(0,&reads);

    while(1){
        temps = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        result = select(1, &temps, 0, 0, &timeout);
        if(result==-1){
            puts("select() error!");
            break;
        }else if(result==0){
            puts("Time-out!");
        }else{
            if(FD_ISSET(0,&temps)){
                str_len = read(0,buf,BUF_SIZE);
                buf[str_len] = 0;
                printf("message from console: %s",buf);
            }
        }
    }
    return 0;
}
```

#### 编译运行

```shell
$ gcc select.c ./bin/select 
$ ./bin/select 
hello
message from console: hello
123
message from console: 123
Time-out!
Time-out!
```



## 4、实现I/O复用服务器端

#### 代码

echo_selectserv.c

```c
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define BUF_SIZE 1024
void errorHandling(const char *message);


int main(int argc, char *argv[])
{
    int str_len, state;
    int serv_sock, clnt_sock;
    char message[BUF_SIZE];
    int fds[2];

    int fd_max, fd_num;
    fd_set reads,cpy_reads;
    struct timeval timeout;


    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    if (argc != 2)
    {
        printf("Usage: %s <port> \n", argv[1]);
        exit(1);
    }

    //调用socket函数创建套接字
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        errorHandling("socket() error");
    }
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    //调用bind函数分配IP地址和端口号
    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("bind() error!");
    }

    //调用listen函数将套接字转化为可接收连接状态
    if (listen(serv_sock, 5) == -1)
    {
        errorHandling("listen() error!");
    }
    //write函数用于传输数据，若程序经accept函数运行到本行，说明已经有了连接请求
    //调用accept函数受理连接请求，如果在没有连接请求的情况下调用该函数，则不会返回，直到有连接请求为止。
   
    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);
    fd_max = serv_sock;
    
    while(1){
        cpy_reads = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;
        if((fd_num=select(fd_max+1, &cpy_reads, 0,0,&timeout))==-1){
            break;
        }

        if(fd_num == 0){
            continue;    //没有消息
        }
        for(int i=0; i<fd_max+1; i++){
            if(FD_ISSET(i,&cpy_reads)){
                if(i==serv_sock){
                    clnt_adr_sz = sizeof(clnt_adr);
                    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
                    FD_SET(clnt_sock, &reads);
                    if(fd_max<clnt_sock){
                        fd_max = clnt_sock;
                    }

                    printf("Connect client: %d\n",clnt_sock);
                }else{
                    str_len = read(i, message, BUF_SIZE);
                    if(str_len==0){
                        FD_CLR(i,&reads);
                        close(i);
                        printf("close client :%d \n",i);
                    }else{
                        write(i,message,str_len);
                    }
                }
            }
        }
    }

    close(serv_sock);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

```

> 当客户端请求与服务器端建立连接时，本质上也是发送数据，因此对于服务器来说，建立连接也按接收数据来算。

客户端可以选择前面章节的任意一个客户端，这里选择第11章的

echo_mpclient.c

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void errorHandling(const char *message);
void read_routine(int sock, char *buf);
void write_routine(int sock, char *buf);
int itoc(int num, char *str);

int main(int argc, char *argv[])
{
    int sock;
    char message[BUF_SIZE];
    int str_len;
    struct sockaddr_in serv_adr;

    pid_t pid;

    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        errorHandling("socket() error");
    }

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("connect() error!");
    }
    else
    {
        puts("Connected......");
    }

    pid = fork();
    if (pid == 0)
    {
        write_routine(sock, message);
    }
    else
    {
        read_routine(sock, message);
    }
    close(sock);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void read_routine(int sock, char *buf)
{
    while (1)
    {
        int str_len = read(sock, buf, BUF_SIZE);
        if (str_len == 0)
            return;

        buf[str_len] = 0;
        printf("Message from server: %s", buf);
    }
}

void write_routine(int sock, char *buf)
{
    while (1)
    {
        fgets(buf, BUF_SIZE, stdin);
        if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n"))
        {
            shutdown(sock, SHUT_WR);
            return;
        }

        write(sock, buf, strlen(buf));
    }
}

int itoc(int num, char *str)
{
    char tem[1024];
    int id = 0, id2 = 0;
    while (num)
    {
        int t = num % 10;
        tem[id++] = t + '0';
        num /= 10;
    }
    str[id--] = '\0';
    while (id >= 0)
    {
        str[id2++] = tem[id--];
    }
    return 0;
}
```

#### 编译运行

服务器端

```shell
$ gcc echo_selectserv.c -o ./bin/echo_selectserv 
$ ./bin/echo_selectserv 9190
Connect client: 4
close client :4 
```

客户端

```shell
$ gcc echo_mpclient.c -o ./bin/echo_mpclient 
$ ./bin/echo_mpclient 127.0.0.1 9190
Connected......
hello world
Message from server: hello world
how are you
Message from server: how are you
q
```



## 5、习题（参考答案）

（1）请解释复用技术的通用含义，并说明何为I/O复用。

> 答：“复用”的通用含义是“为了提高物理设备的效率，用最少的物理要素传递最多数据时使用的技术”
>
> I/O复用可以理解为用一个进程为多个客户提供服务。

（2）多进程并发服务器的缺点有哪些？如何在I/O复用服务端中弥补？

> 答：创建进程需要付出极大代价，需要大量的运算和内存空间，进程间通信也比较复杂。
>
> 可以使用select函数，用一个进程为多个客户端提供服务。

（3）复用服务器端需要select函数。下列关于select函数使用方法的描述错误的是？

> 答：c
>
> a正确，调用select函数前需要将集中I/O监视对象的文件描述符
>
> b正确。d正确

（4）select函数的观察对象中应包含服务器端套接字（监听套接字），那么应将其包含到哪一类监听对象集合？请说明原因？

> 答：readset集合，当客户端请求与服务器端建立连接时，本质上也是发送数据，因此对于服务器来说，建立连接即接收数据。

（5）略

# 第 13 章 多种I/O函数

之前的章节中，在Linux环境下使用read & write函数完成数据I/O。本章将介绍send & recv及其他I/O函数。

## 1、send & recv函数

send 函数声明

```c
#include <sys/socket.h>

ssize_t send(int sockfd, const void *buf, size_t nbytes, int flags);
// 成功时返回发送的字节数，失败时返回-1

/*
	sockfd 表示数据传输对象连接的套接字文件描述符
	buf 保存待传输数据的缓冲地址值
	nbytes 待传输的字节数
	flags 传输数据时指定的可选项信息
*/
```



recv 函数声明

```c
#include <sys/socket.h>

ssize_t recv(int sockfd, void* buf, size_t nbytes, int flags);
//成功时返回接收的字节数（EOF时返回0），失败时返回-1

/*
	sockfd 表示数据接收对象连接的套接字文件描述符
	buf 保存接收数据的缓冲地址值
	nbytes 可接收的最大字节数
	flag 接收数据时指定的可选项信息
*/
```

send & recv 函数可选项及含义

| 可选项(option) | 含义                                                         | send | recv |
| -------------- | ------------------------------------------------------------ | ---- | ---- |
| MSG_OOB        | 用于传输带外数据(Out-of-band data)                           | Y    | Y    |
| MSG_PEEK       | 验证输入缓冲是否存在接收的数据                               | N    | Y    |
| MSG_DONTROUTE  | 传输传输过程中不参照路由(Routing)表，在本地(Local)网络中寻找目的地 | Y    | N    |
| MSG_DONTWAIT   | 调用I/O函数时不阻塞，用于使用非阻塞(Non-blocking)I/O         | Y    | Y    |
| MSG_WAITALL    | 防止函数返回，直到接收全部请求的字节数                       | N    | Y    |

多个可选项用 |(位或运算符) 连接，例如：  MSG_OOB|MSG_PEEK

不同操作系统对上述可选项的支持也不同，因此，为了使用不同可选项，在实际开发中需要对采用的操作系统有一定了解。



### 1.1 MSG_OOB 发送紧急消息

#### 示例代码

服务器端：

oob_recv.c

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#define BUF_SIZE 30
void error_handling(char *message);
void urg_handler(int signo);

int acpt_sock;
int recv_sock;

int main(int argc, char *argv[])
{
    struct sockaddr_in recv_adr, serv_adr;
    int str_len, state;
    socklen_t serv_adr_sz;
    struct sigaction act;
    char buf[BUF_SIZE];
    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    act.sa_handler = urg_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    acpt_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&recv_adr, 0, sizeof(recv_adr));
    recv_adr.sin_family = AF_INET;
    recv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    recv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(acpt_sock, (struct sockaddr *)&recv_adr, sizeof(recv_adr)) == -1)
        error_handling("bind() error");
    listen(acpt_sock, 5);

    
    while(1){
        serv_adr_sz = sizeof(serv_adr);
        recv_sock = accept(acpt_sock, (struct sockaddr *)&serv_adr, &serv_adr_sz);
        //将文件描述符 recv_sock 指向的套接字拥有者（F_SETOWN）改为把getpid函数返回值用做id的进程
        fcntl(recv_sock, F_SETOWN, getpid());
        state = sigaction(SIGURG, &act, 0); //SIGURG 是一个信号，当接收到 MSG_OOB 紧急消息时，系统产生SIGURG信号
        while ((str_len = recv(recv_sock, buf, BUF_SIZE-1, 0))!=0)
        {
            if (str_len == -1)
                continue;
            buf[str_len] = 0;
            puts(buf);
        }
        close(recv_sock);
    }
    close(acpt_sock);
    return 0;
}
void urg_handler(int signo)
{
    int str_len;
    char buf[BUF_SIZE];
    str_len = recv(recv_sock, buf, sizeof(buf) - 1, MSG_OOB);
    buf[str_len] = 0;
    printf("Urgent message: %s \n", buf);
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
```

收到MSG_OOB紧急信号时，操作系统将产生SIGURG信号，并调用注册的信号处理函数

fcnt 将文件描述符recv_sock指向的套接字拥有者(F_SETOWN)改为PID为getpid()返回值的进程。



oob_send.c

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 30
void error_handling(char *message);

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in recv_adr;
    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }
    sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&recv_adr, 0, sizeof(recv_adr));
    recv_adr.sin_family = AF_INET;
    recv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    recv_adr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&recv_adr, sizeof(recv_adr)) == -1)
        error_handling("connect() error");

    write(sock, "123", strlen("123"));
    send(sock, "4", strlen("4"), MSG_OOB);
    write(sock, "567", strlen("567"));
    send(sock, "890", strlen("890"), MSG_OOB);
    close(sock);
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
$ gcc oob_recv.c -o ./bin/oob_recv 
$ ./bin/oob_recv 9190
-----------------
new client : 4
123456789
-----------------
new client : 4
123456789
-----------------
new client : 4
123456789
-----------------
new client : 4
123456789
-----------------
new client : 4
123456789
-----------------
new client : 4
Urgent message: 0 
123
56789
-----------------
new client : 4
123
Urgent message: 0 
56789
-----------------
new client : 4
123456789
-----------------
new client : 4
123456789
-----------------
new client : 4
123456789
-----------------
new client : 4
123
Urgent message: 0 
56789
-----------------
new client : 4
123
Urgent message: 4 
567
Urgent message: 0 
89
-----------------
new client : 4
123456789
-----------------
new client : 4
123456789
-----------------
new client : 4
Urgent message: 0 
123
56789
-----------------
new client : 4
123456789
-----------------
new client : 4
123456789
-----------------
new client : 4
123456789
-----------------
new client : 4
123456789
-----------------
new client : 4
123
567
Urgent message: 0 
89
```

> 有时能产生URG信号，有时不能，MSG_OOB的真正意义在于督促数据接收对象尽快处理数据，如果数据已经接收就不在产生URG信号



```shell
$ gcc oob_send.c -o ./bin/oob_send 
$ ./bin/oob_send 127.0.0.1 9190
$ ./bin/oob_send 127.0.0.1 9190
$ ./bin/oob_send 127.0.0.1 9190
$ ./bin/oob_send 127.0.0.1 9190
$ ./bin/oob_send 127.0.0.1 9190
$ ./bin/oob_send 127.0.0.1 9190
$ ./bin/oob_send 127.0.0.1 9190
。。。
$ ./bin/oob_send 127.0.0.1 9190
```



### 1.2 紧急模式工作原理

**MSG_OOB的真正意义在于督促数据接收对象尽快处理数据**。这是紧急模式的全部内容，而且TCP保持顺序传输"的传输特性依然成立。

使用带外数据的实际程序例子就是telnet,rlogin,ftp命令。前两个程序会将中止字符作为紧急数据发送到远程端。这会允许远程端冲洗所有未处理 的输入，并且丢弃所有未发送的终端输出。这会快速中断一个向我们屏幕发送大量数据的运行进程。ftp命令使用带外数据来中断一个文件的传输。

举例：急珍患者的及时救治需要如下两个条件：

* 迅速入院
* 医院急救

无法快速把病人送到医院，并不意味着不需要医院进行急救。TCP的急救消息无法保证及时入院，但可以要求急救。当然，急救措施由程序员完成。之前的示例oob_recv.c的运行过程中也传递了紧急消息，这可以通过事件处理函数确认。这就是MSG_OOB模式数据传输的实际意义。下面给出设置MSG_OOB可选项状态下的数据传输过程，如下图所示：

<img src="https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-10%20%E4%B8%8B%E5%8D%889.54.06.png" alt="截屏2021-05-10 下午9.54.06" style="zoom:50%;" />

上图给出oob_recv.c调用send(sock, "890", strlen("890"), MSG_OOB)后输出缓冲状态，假设已传输之前的数据。

如果将缓冲最左端的位置视为偏移量为0，字符0保存于偏移量为2的位置。另外，字符0右侧偏移量为3的位置存有紧急指针。紧急指针指向紧急消息的下一个位置（偏移量加一），同时向对方主机传递如下信息：

“紧急指针指向的偏移量为3的之前的部分就是紧急消息！”

也就是，只用一个字节表示紧急消息，这一点可通过上图用于传输数据的TCP数据包的部分结构看的更清楚。如下图所示：

<img src="https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-10%20%E4%B8%8B%E5%8D%8810.23.49.png" alt="截屏2021-05-10 下午10.23.49" style="zoom: 50%;" />

TCP头中含有如下两种信息：

* URG=1:载有紧急消息的数据包
* URG指针：紧急指针位于偏移量为3的位置

指定MSG_OOB选项的数据包本身就是紧急数据包，并通过紧急指针表示紧急消息所在位置，但如图13-2无法得知以下事实：

“紧急消息是字符串890，还是90？如若不是，是否为单个字节0？”

这些都不重要，除紧急指针的前面1个字符，数据接收方将通过调用常用输入函数读取剩余部分。

### 1.3 检查输入缓冲

同时设置MSG_PEEK选项和MSG_DONTWAIT选项，以验证输入缓冲中是否存在接收的数据。MSG_PEEK选项，即使读取了缓冲中的数据也不会删除，加上额外的选项MSG_DONTWAIT，以非阻塞方式验证。

#### 示例代码

peek_recv.c

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void error_handling(char *message);

int main(int argc, char* argv[]){
    int sock;
    struct sockaddr_in send_adr;
    if(argc!=3){
        printf("Usage %s <IP> <PORT>",argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&send_adr, 0 , sizeof(send_adr));
    send_adr.sin_family = AF_INET;
    send_adr.sin_addr.s_addr = inet_addr(argv[1]);
    send_adr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*)&send_adr,sizeof(send_adr)) ==-1 ){
        error_handling("connect() error");
    }

    write(sock, "123", 3);
    close(sock);
    return 0;
}

void error_handling(char * message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}
```



peek_send.c

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void error_handling(char *message);

int main(int argc, char* argv[]){
    int sock;
    struct sockaddr_in send_adr;
    if(argc!=3){
        printf("Usage %s <IP> <PORT>",argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&send_adr, 0 , sizeof(send_adr));
    send_adr.sin_family = AF_INET;
    send_adr.sin_addr.s_addr = inet_addr(argv[1]);
    send_adr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*)&send_adr,sizeof(send_adr)) ==-1 ){
        error_handling("connect() error");
    }

    write(sock, "123", 3);
    close(sock);
    return 0;
}

void error_handling(char * message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}
```



#### 编译运行

```shell
$ gcc peek_recv.c -o ./bin/peek_recv
$ ./bin/peek_recv 9190
Buffering 3 bytes: 123 
Read again: 123 
```

```shell
$ gcc peek_send.c -o ./bin/peek_send
$ ./bin/peek_send 127.0.0.1 9190
```



可见仅发送一次的数据，被服务器端读取了两次，第一次调用设置了MSG_PEEK选项，用于检查，读取后并不删除缓冲区的数据。



## 2、readv & writev 函数

功能：对数据进行整合传输及发送的函数，通过writev函数可以将分散保存在多个缓冲中的数据一并发送，通过readv函数可以由多个缓冲分别接收。因此，适当使用这2个函数可以介绍I/O函数的调用次数。

### 2.1 writev函数

writev函数的声明

```c
#include <sys/uio.h>

ssize_t writev(int filedes, const struct iovec *iov, int iocvnt);
//成功时返回发送的字节数，失败时返回-1

/*
	filedes 文件描述符，可以是套接字，也可以是普通文件描述符
	iov iovec结构体数据的地址值，结构体iovec中包含待发送数据的位置和大小信息
	iovcnt 向第二个参数传递的数组长度
*/
```

iovec结构体

```c
struct iovec {
	void * iov_base; //缓冲地址
  size_t iov_len;  //缓冲大小
}
```

writev函数的功能如下图所示：

<img src="https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-11%20%E4%B8%8A%E5%8D%8811.06.30.png" alt="截屏2021-05-11 上午11.06.30" style="zoom: 67%;" />



#### 示例代码

writev.c

```c
#include <stdio.h>
#include <sys/uio.h>
#include <string.h>

int main(int argc, char * argv[]){
    struct iovec vec[2];
    char buf1[] = "ABCDEF";
    char buf2[] = "123456";

    int str_len;

    vec[0].iov_base = buf1;
    vec[0].iov_len = strlen(buf1);

    vec[1].iov_base = buf2;
    vec[1].iov_len = strlen(buf2);

    str_len = writev(1, vec, 2);
    puts("");
    printf("Write bytes: %d \n", str_len);
    return 0;
}
```



#### 编译运行

```shell
$ gcc writev.c -o ./bin/writev
$ ./bin/writev 
ABCDEF123456
Write bytes: 12 
```



### 2.2 readv函数

```c
#include <sys/uio.h>

ssize_t readv(int filedes, const struct iovec * iov, int iovcnt);
// 成功时返回接收的字节数，失败时返回-1

/*
	filedes 文件描述符，可以是套接字，也可以是普通文件描述符
	包含数据保存位置和大小信息的iovec结构体数组的地址值
	iovcnt 第二个参数中数组的长度
*/
```



#### 示例代码

readv.c

```c
#include <stdio.h>
#include <sys/uio.h>

#define BUF_SIZE 100

int main(int argc, char * argv[]) {
    struct iovec vec[2];
    char buf1[BUF_SIZE] = {0,};
    char buf2[BUF_SIZE] = {0, };
    int str_len;

    vec[0].iov_base  = buf1;
    vec[0].iov_len = 5;

    vec[1].iov_base = buf2;
    vec[1].iov_len = BUF_SIZE;

    str_len = readv(0, vec, 2);
    printf("Read bytes: %d \n", str_len);
    printf("First message: %s \n", buf1);
    printf("Second message: %s", buf2);
    return 0;
}
```



#### 编译运行

```shell
$ gcc readv.c -o ./bin/readv
$ ./bin/readv
I like TCP/IP socket programming~
Read bytes: 34 
First message: I lik 
Second message: e TCP/IP socket programming~
```

Second message已经包含一个换行符，所以不用在printf里输出换行符



### 2.3 合理使用readv & writev 函数

需要传输到的数据位于不同缓冲时，需要多次调用write函数，此时可以通过一次writev调用完成操作，会提高效率。同样，需要将输入缓冲中的数据读入不同位置时，可以不必多次调用read函数，而是利用一次调用readv函数就能大大提高效率。

仅从c语言角度，减少函数调用能相应地提高性能，但其更大意义在于减少数据包个数。假设为了提高效率而在服务器端明确阻止使用Nagle算法。其实writev函数再不采用Nagle算法时更有价值。

<img src="https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-11%20%E4%B8%8A%E5%8D%8811.40.18.png" alt="截屏2021-05-11 上午11.40.18" style="zoom: 33%;" />

为了提高速度而关闭了Nagle算法，如果调用3次write函数，很有可能通过3个数据包传输，而调用1次writev函数，只需通过1个数据包传输。



需要将不同位置的数据按照发送顺序移动到1个大数组，并通过1次write函数调用进行传输。这种方式，可以直接使用writv函数一次完成。



## 3、习题（参考答案）

（1）下列关于MSG_OOB可选项的说法错误的是？

a. MSG_OOB 指传输 Out-of-band 数据，是通过其他路径高速传输数据

b. MSG_OOB 指通过其他路径高速传输数据，因此 TCP 中设置该选项的数据先到达对方主机

c. 设置 MSG_OOB 是数据先到达对方主机后，以普通数据的形式和顺序读取。也就是说，只是提高了传输速度，接收方无法识别这一点。
d. MSG_OOB 无法脱离 TCP 的默认数据传输方式，即使脱离了 MSG_OOB ，也会保持原有的传输顺序。该选项只用于要求接收方紧急处理。

> 答：b，MSG_OOB的真正意义是督促数据接收对象尽快处理数据，且TCP"保持传输顺序"的传输特性依然成立。
>
> c：MSG_OOB并不会提高传输速度

（2）利用readv & writev函数收发数据有何优点？分别从函数调用次数和I/O缓冲的角度给出说明。

> 答：writev可以将多个缓冲的数据一并发送，readv，可以由多个缓冲一起接收数据。使用readv & writev函数可以减少I/O函数的调用次数。
>
> 当为了提高数据传输速度而关闭Nagle算法时，使用readv & writev函数的优势更加明显，需要发送3个不同缓冲区的内容时，使用write可能需要发送3个数据包，使用writev可能只需发送1个数据包。

（3）通过recv函数验证输入缓冲是否存在数据时（确认后立即返回），如何设置recv函数最后一个参数中的可选项？分别说明各可选项的含义？

> 答：可选项为MSG_PEEK|MSG_DONTWAIT，MSG_PEEK为即使读取了输入缓冲的数据也不会删除，MSG_DONTWAIT为以非阻塞方式验证待读数据存在与否。

# 第 14 章 多播与广播

## 1、多播

### 1.1 多播的数据传输方式及流量方面的优点

多播数据传输的特点

* 多播服务器器端针对特定多播组，只发送一次数据
* 即使只发送一次数据，但改组内的所有客户端都会接收数据
* 多播组数可在IP地址范围内任意增加
* 加入特定组即可接收发往该多播组的数据。

多播组是D类IP地址(224.0.0.0-239.255.255.255）

### 1.2 路由(Routing) TTL(Time to Live, 生存时间) 及加入组的办法

 TTL指数据包传输距离，每经过一个数据包，其值减1，TTL变为0时，将不在被传递



设置TTL通过第9章套接字的可选项完成，与TTL相关的协议层是IPPROTO_IP，选项名为 IP_MULTICAST_TTL，用如下代码设置TTL

```c
int send_sock;
int time_live = 64;
...
send_sock=socket(PF_INET, SOCK_DGRAM, 0);
setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, , (void*)&time_live, sizeof(time_live));
...
```



加入多播组也通过设置套接字可选项完成，加入多播组的协议层是IPPROTO_IP，选项名IP_ADD_MEMBERSHIP,通过下列代码加入多播组

```c
int recv_sock;
struct ip_mreq join_adr;
...
recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
...
join_adr.imr_multiaddr.s_addr="多播组地址信息";
join_adr.imr_interface.s_addr="加入多播组的主机地址信息";
setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr)); 
```



Ip_mreq结构体定义

```c
struct ip_mreq{
  struct in_addr imr_multiaddr;     //加入的组IP地址
  struct in_addr imr_interface;     //加入该组的套接字所属主机的IP地址，也可使用INADDR_ANY
}
```



### 1.3 实现多播 Sender 和 Receiver

该示例的运行场景如下：

* Sender：向AAA组（Broadcastin）文件中保存的新闻信息
* Receiver：接收传递到AAA组的新闻信息



news_sender.c

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define TTL 64
#define BUF_SIZE 70

void error_handling(char *message);
int main(int argc, char*argv[]){
    int send_sock;
    struct sockaddr_in mul_adr;
    int time_live = TTL;
    int str_len;
    FILE *fp;
    char buf[BUF_SIZE];
    if (argc != 3)
    {
        printf("Usage %s <GroupIP> <PORT>", argv[0]);
        exit(1);
    }

    send_sock = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&mul_adr, 0, sizeof(mul_adr));
    mul_adr.sin_family=AF_INET;
    mul_adr.sin_addr.s_addr = inet_addr(argv[1]);
    mul_adr.sin_port = htons(atoi(argv[2]));

    setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&time_live, sizeof(time_live));
    if((fp=fopen("news.txt","r")) == NULL )
        error_handling("fopen() error");
    // news.txt 最后一行需多一个换行，不然接收端无法正常输出
    while(1){
        if(fgets(buf,BUF_SIZE-1, fp) == NULL){
            break;
        }
        sendto(send_sock, buf, strlen(buf), 0, (struct sockaddr*)&mul_adr, sizeof(mul_adr));
    }


    fclose(fp);
    close(send_sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
```



news_receive.c

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define BUF_SIZE 30

void error_handling(char *message);

int main(int argc, char*argv[]){
    int recv_sock;
    int str_len;
    char buf[BUF_SIZE];
    struct sockaddr_in adr;
    struct ip_mreq join_adr;

    if (argc != 3)
    {
        printf("Usage %s <GroupIP> <PORT>", argv[0]);
        exit(1);
    }

    recv_sock=socket(PF_INET, SOCK_DGRAM, 0);
    memset(&adr, 0 , sizeof(adr));
    adr.sin_family = AF_INET;
    adr.sin_addr.s_addr = htonl(INADDR_ANY);
    adr.sin_port = htons(atoi(argv[2]));

    if(bind(recv_sock, (struct sockaddr*)&adr, sizeof(adr))==-1)
        error_handling("bind() error");
    
    join_adr.imr_multiaddr.s_addr = inet_addr(argv[1]);
    join_adr.imr_interface.s_addr = htonl(INADDR_ANY);

    setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr));

    while(1){
        str_len = recvfrom(recv_sock, buf, BUF_SIZE-1, 0, NULL, 0);
        if(str_len<0)
            break;
        buf[str_len]=0;
        fputs(buf, stdout);
    }

    close(recv_sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
```

news.txt最后一行要有换行，news_receive先运行，news_sender后运行

编译运行

```shell
$ gcc news_receiver.c -o ./bin/news_receiver
$ ./bin/news_receiver 224.1.1.2 9190
这是1条新闻：
新闻的主要内容为：
one
two
three
2021-1-1
```



```shell
$ gcc news_sender.c -o ./bin/news_sender 
$ ./bin/news_sender 224.1.1.2 9190
```



多播是基于MBone这个虚拟网络工作的，可以将其理解为：“通过网络中的特殊协议工作的软件概念上的网络”。也就是说，MBone并非可以触及的物理网络，它是以物理网络为基础，通过软件方法实现的多播通信必备虚拟网络。



## 2、广播

多播即使跨越不同网络的情况下，只要加入多播组就能接收数据。相反，广播只能向同一网络中的主机传输数据。

### 2.1 广播的理解及实现方法

广播是向同一网络中所有主机传输数据的方法。与多播相同，广播也是基于UDP完成的。根据传输数据时使用IP地址的形式，广播分为如下2种。

* 直接广播（Directed Broadcast）
* 本地广播（Local Broadcast）

直接广播的IP地址：除了网络地址，主机地主全部置为1。例如192.32.24网络中的主机向192.32.24.255传输数据时，数据将传递到192.32.24网络中所有主机。

本地广播使用的IP地址为255.255.255.255，例如，192.32.24网络中的主机向255.255.255.255传输数据时，数据将传递到192.32.24网络中所有主机。



通过修改套接字的可选项，使其支持广播。

```c
int send_sock;
int bcast = 1;    //对变量进行初始化，以将SO_BROADCAST选项信息改为1
...
send_sock = socket(PF_INET, SOCK_DGRAM, 0);
...
setsockopt(send_sock, SOL_SOCKET, SO_BROADCAST, (void*)&bcast, sizeof(bcast));
...

```



### 2.2 实现广播数据的Sender和Receiver

news_sender_brd.c

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 30

void error_handling(char *message);

int main(int argc, char*argv[]){
    int send_sock;
    struct sockaddr_in broad_adr;
    FILE *fp;
    char buf[BUF_SIZE];
    int so_brd = 1;
    if (argc != 3)
    {
        printf("Usage %s <GroupIP> <PORT>", argv[0]);
        exit(1);
    }

    send_sock = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&broad_adr, 0, sizeof(broad_adr));
    broad_adr.sin_family = AF_INET;
    broad_adr.sin_addr.s_addr = inet_addr(argv[1]);
    broad_adr.sin_port = htons(atoi(argv[2]));

    setsockopt(send_sock, SOL_SOCKET, SO_BROADCAST, (void *)&so_brd, sizeof(so_brd));
    if ((fp = fopen("news.txt", "r")) == NULL)
        error_handling("fopen() error");
    // news.txt 最后一行需多一个换行，不然接收端无法正常输出
    while (1)
    {
        if (fgets(buf, BUF_SIZE - 1, fp) == NULL)
            break;
        
        sendto(send_sock, buf, strlen(buf), 0, (struct sockaddr *)&broad_adr, sizeof(broad_adr));
    }

    fclose(fp);
    close(send_sock);
    return 0;
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
```



news_receiver_brd.c

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define BUF_SIZE 30

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int recv_sock;
    int str_len;
    char buf[BUF_SIZE];
    struct sockaddr_in adr;

    if (argc != 2)
    {
        printf("Usage %s <PORT>", argv[0]);
        exit(1);
    }

    recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&adr, 0, sizeof(adr));
    adr.sin_family = AF_INET;
    adr.sin_addr.s_addr = htonl(INADDR_ANY);
    adr.sin_port = htons(atoi(argv[1]));

    if (bind(recv_sock, (struct sockaddr *)&adr, sizeof(adr)) == -1)
        error_handling("bind() error");

    while (1)
    {
        str_len = recvfrom(recv_sock, buf, BUF_SIZE - 1, 0, NULL, 0);
        if (str_len < 0)
            break;
        buf[str_len] = 0;
        fputs(buf, stdout);
    }

    close(recv_sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
```



编译运行

news.txt最后一行要有换行，news_receive先运行，news_sender后运行

```shell
$ gcc news_receiver_brd.c -o ./bin/news_receiver_brd
$ ./bin/news_receiver_brd 9190这是1条新闻：
新闻的主要内容为：
one
two
three
2021-1-1
```

```shell
$ gcc news_sender_brd.c -o ./bin/news_sender_brd
$ ./bin/news_sender_brd 255.255.255.255 9190
```



## 3、习题（参考答案）

（1）TTL的含义是什么？请从路由的角度说明较大的TTL值与较小的TTL值之间的区别及问题。

> TTL值指生存时间，即经过路由器的个数。没经过一个路由器，其TTL值减1，直至TTL变为0就不再传输。
>
> 较大的TTL值相比较小的TTL值能传输更远的距离，但太大会影响网络流量。一般设置为64.



（2）多播与广播的异同点是什么？请从数据通信的角度进行说明。

> 答：多播与广播的相同点就是都能用1个数据包，同时向多台主机发送数据。
>
> 不同点：多播即使跨越不同网络的情况下，只要加入多播组就能接收数据。相反，广播只能向同一网络中的主机传输数据。



（3）下列关于多播的描述错误的是？

a. 多播就是用来向加入多播组的所有主机传输数据的协议。

b. 主机连接到同一网络才能加入多播组，也就是说多播组无法跨越多个网络。

c. 能够加入多播组的主机数并无限制，但只能有一个主机（Sender）向该组发送数据。

d. 多播时使用的套接字是UDP套接字，因为多播是基于UDP进行数据通信的。

> 答：b: 不同网络的主机也可以加入同一个多播组
>
> ​        c：可以有多个主机向该组发送数据



（4）多播也对网络流量有利，请比较TCP数据交换方式解释其原因。

> 答：若通过TCP或UDP向1000个主机发送文件，则共需传递1000次，即便将10台主机合并为1个网络，使99%的传输路径相同的情况下也是如此。若使用多播方式传输文件，则只需发送一次。这时由1000台主机构成的网络中的路由器负责复制文件并传递到主机。



（5）多播方式的数据通信需要MBone虚拟网络。换言之，MBone是用于多播的网络，但它是虚拟网络。请解释此处的虚拟网络。

> 答：MBone是“通过网络中的特殊协议工作的软件概念上的网络”。也就是说，MBone并非可以触及的物理网络，它是以物理网络为基础，通过软件方法实现的多播通信必备虚拟网络。# 第 15 章 套接字和标准I/O

## 1、标准I/O函数的优点

### 1.1 标准I/O函数的两大优点

* 标准I/O函数具有良好的移植性（Portability）。
* 标准I/O函数可以利用缓冲提高性能。

在进行TCP通信时，若使用标准I/O函数，将产生以下两种缓冲。

<img src="https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-15%20%E4%B8%8B%E5%8D%888.02.54.png" alt="截屏2021-05-15 下午8.02.54" style="zoom:50%;" />

使用标准I/O函数缓冲的目的主要是为了提高性能。缓冲并非在所有情况下都能带来卓越的性能。但需要传输的数据越多，有无缓冲带来性能差异越大。可以通过以下两种角度说明性能的提高。

* 传输的数据量
* 数据向输出缓冲移动的次数。

假设一个TCP数据包头信息占 40 个字节：

* 1 个字节10次 40*10=400字节
* 10字节 1次 40*1=40字节

使用I/O缓冲，能显著减少数据包的个数，提高数据传输效率。



### 1.2 标准I/O函数和系统函数之间的性能对比

现编写程序实现以下功能：

将news.txt中的内容，复制到cry.txt中，为使对比更加明显，news.txt的大小应该在300M以上。

使用系统函数实现如下：

```c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#define BUF_SIZE 3  // 用最短数组长度构成

int main(int argc, char* argv[]){
    int fd1, fd2;   //保存在fd1和fd2中的是文件描述符
    int len;
    char buf[BUF_SIZE];
    clock_t start,end;
    double duration;   // 计算执行时间

    fd1 = open("news.txt", O_RDONLY);
    fd2 = open("cpy.txt", O_WRONLY | O_CREAT | O_TRUNC);
    start = clock();
    while((len=read(fd1,buf,sizeof(buf)))>0){
        write(fd2,buf,len);
    }
    end=clock();
    duration = (double)(end - start) / CLOCKS_PER_SEC;
    printf("running time: %f seconds\n", duration);

    close(fd1);
    close(fd2);
    return 0;
}
```



编译运行

```shell
$ gcc syscpy.c -o ./bin/syscpy 
$ ./bin/syscpy 
running time: 1.183949 seconds
```

可以看到，使用系统函数执行时间为1.18秒



使用标准I/O函数

stdcpy.c

```c
#include <stdio.h>
#include <time.h>
#define BUF_SIZE 3 //用最短长度构成

int main(int argc, char* argv[]){
    FILE *fp1;
    FILE *fp2;
    time_t start,end;
    double duration;

    char buf[BUF_SIZE];

    fp1 = fopen("news.txt","r");
    fp2 = fopen("cpy.txt", "w");
    
    start = clock();
    while(fgets(buf,BUF_SIZE, fp1)!=NULL)
        fputs(buf,fp2);
    end = clock();

    duration = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Running time: %f 秒\n",duration);

    fclose(fp1);
    fclose(fp2);
    return 0;
}
```

编译运行

```shell
$ gcc stdcpy.c -o ./bin/stdcpy 
$ ./bin/stdcpy 
Running time: 0.040688 秒
```

可以看到，使用标准I/O函数只用了0.04 秒。



### 1.3 标准I/O函数的几个缺点

* 不容易进行双向通信
* 有时可能频繁调用fflush()函数
* 需要以FILE结构体指针的形式返回文件描述符

在使用标准I/O函数时， 每次切换读写状态时应调用fflush函数，这也会影响基于缓冲的性能提高



## 2、使用标准 I/O 函数

### 2.1 利用 fdopen 函数转换为FILE结构体指针

使用以下函数，将文件描述符转换为FILE结构体指针

```c
#include <stdio.h>

FILE *fdopen(int fildes, const char * mode);
// 成功时返回转换的FILE结构体指针，失败时返回NULL

/*
	fildes 需要转换的文件描述符
	mode 将创建的FILE结构体指针的模式信息
*/
```



desto.c

```c
#include <stdio.h>
#include <fcntl.h>

int main(int argc, char* argv[]){
    FILE *fp;
    // S_IRWXU 给文件所属用户赋予读写执行权限
    int fd = open("data.dat", O_WRONLY | O_CREAT | O_TRUNC,S_IRWXU);
    if(fd==-1){
        fputs("file open error",stdout);
        return -1;
    }

    fp=fdopen(fd,"w");
    fputs("Network c programming \n", fp);
    fclose(fp);
    return 0;
}
```



编译运行

```shell
$ gcc desto.c -o ./bin/desto 
$ ./bin/desto 
$ cat data.dat 
Network c programming 
```



### 2.2 利用 fileno 函数转换文件描述符

```c
#include <stdio.h>

int fileno(FILE * stream);
// 成功时返回转换后的文件描述符，失败时返回-1
```



todes.c

```c
#include <stdio.h>
#include <fcntl.h>

int main(int argc, char* argv[]){
    FILE *fp;
    int fd = open("data.dat", O_WRONLY|O_CREAT|O_TRUNC);
    if (fd == -1)
    {
        fputs("file open error", stdout);
        return -1;
    }

    printf("First file descriptor: %d \n", fd);
    fp = fdopen(fd,"w");
    fputs("Network c programming \n",fp);
    printf("Second file descriptor: %d \n",fileno(fp));
    fclose(fp);
    return 0;
}
```



编译运行

```shell
$ gcc todes.c -o ./bin/todes 
$ ./bin/todes 
First file descriptor: 3 
Second file descriptor: 3 
```



## 3、基于套接字的标准 I/O 函数使用

使用第4章的echo_server.c和echo_client.c 进行简单修改，只需将代码中的文件描述符，改为FILE结构体指针即可。

echo_stdserv.c

```c
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024
void errorHandling(const char *message);

int main(int argc, char *argv[])
{
    int str_len;
    int serv_sock, clnt_sock;
    char message[BUF_SIZE];

    FILE *readfp;
    FILE *writefp;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    if (argc != 2)
    {
        printf("Usage: %s <port> \n", argv[1]);
        exit(1);
    }

    //调用socket函数创建套接字
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        errorHandling("socket() error");
    }
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    //调用bind函数分配IP地址和端口号
    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("bind() error!");
    }

    //调用listen函数将套接字转化为可接收连接状态
    if (listen(serv_sock, 5) == -1)
    {
        errorHandling("listen() error!");
    }
    clnt_adr_sz = sizeof(clnt_adr);
    //write函数用于传输数据，若程序经accept函数运行到本行，说明已经有了连接请求
    //调用accept函数受理连接请求，如果在没有连接请求的情况下调用该函数，则不会返回，直到有连接请求为止。
    for (int i = 0; i < 5; i++)
    {
        //调用accept函数受理连接请求，如果在没有连接请求的情况下调用该函数，则不会返回，直到有连接请求为止。
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
        if (clnt_sock == -1)
            errorHandling("accept() error!");
        else
            printf("connected client %d \n",i+1);
        
        readfp = fdopen(clnt_sock,"r");
        writefp = fdopen(clnt_sock, "w");
        while(!feof(readfp)){
            fgets(message, BUF_SIZE, readfp);
            fputs(message, writefp);
            fflush(writefp);    // 将数据立即传输给客户端
        }
        fclose(readfp);
        fclose(writefp);
    }
    close(serv_sock);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
```



echo_client.c

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void errorHandling(const char *message);

int main(int argc, char *argv[])
{
    int sock;
    char message[BUF_SIZE];
    int str_len;
    struct sockaddr_in serv_adr;

    FILE *readfp;
    FILE *writefp;

    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        errorHandling("socket() error");
    }

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("connect() error!");
    }
    else
    {
        puts("Connected......");
    }

    readfp = fdopen(sock, "r");
    writefp = fdopen(sock, "w");
    while (1)
    {
        fputs("Input message(Q to quit):", stdout);
        fgets(message, BUF_SIZE, stdin);
        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
            break;
        
        fputs(message,writefp);
        fflush(writefp);
        fgets(message, BUF_SIZE, readfp);
        // 使用标准I/O函数，可以按字符串单位进行数据交换，因此不用在数据的尾部插入0
        printf("Message from server: %s", message);
    }

    close(sock);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
```



编译运行

```shell
$ gcc echo_stdserv.c -o ./bin/echo_stdserv
$ ./bin/echo_stdserv 9190
connected client 1 
```



```shell
$ gcc echo_client.c -o ./bin/echo_client
$ ./bin/echo_client 127.0.0.1 9190
Connected......
Input message(Q to quit):hello
Message from server: hello
Input message(Q to quit):world
Message from server: world
Input message(Q to quit):123
Message from server: 123
Input message(Q to quit):abc
Message from server: abc
Input message(Q to quit):
```



## 4、习题（参考答案）

（1）请说明标准I/O函数的2个优点。它为何拥有这2个优点？

> 答：标准I/O函数具有良好的移植性（Portability）。标准I/O函数可以利用缓冲提高性能。
>
> 因为标准I/O是按照ANSI C标准定义的，支持大部分操作系统。
>
> 在传输的数据量很大时，利用I/O缓冲可以减少数据交换次数，提高数据传输效率。

（2）利用标准I/O函数传输数据时，下面的想法是错误的：

“调用fputs函数传输数据时，调用后应立即开始发送！”

为何说上述说法是错误的？为了达到这种效果应添加哪些处理过程？

> 答：fputs无法保证立即传输数据，使用fflush()函数来立即发送数据

# 第 16 章 关于I/O流分离的其他内容

## 1、分离I/O流

分离“流”的好处

第10章

* 通过分开输入过程和输出过程降低实现难度
* 与输入无关的操作可以提高速度

第15章

* 为了将FILE指针按读模式和写模式加以区分
* 通过区分读写模式降低实现难度
* 通过区分I/O缓冲提高性能

“流”分离的方法，情况（目的）不同时，带来的好处也有所不同



## 2、“流”分离带来的EOF问题

第15章通过区分FILE指针的读写模式来分离“流”。这样的实现方式如果单纯地调用fclose函数来传递EOF会产生问题，下面用程序验证：

sep_serv.c

```c
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024
void errorHandling(const char *message);

int main(int argc, char *argv[])
{
    int str_len;
    int serv_sock, clnt_sock;
    char buf[BUF_SIZE]={0,};

    FILE *readfp;
    FILE *writefp;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    if (argc != 2)
    {
        printf("Usage: %s <port> \n", argv[1]);
        exit(1);
    }

    //调用socket函数创建套接字
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        errorHandling("socket() error");
    }
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    //调用bind函数分配IP地址和端口号
    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("bind() error!");
    }

    //调用listen函数将套接字转化为可接收连接状态
    if (listen(serv_sock, 5) == -1)
    {
        errorHandling("listen() error!");
    }
    clnt_adr_sz = sizeof(clnt_adr);

    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);

    readfp = fdopen(clnt_sock, "r");
    writefp = fdopen(clnt_sock, "w");

    fputs("From server: Hi,client? \n",writefp);
    fputs("I love all of the world \n",writefp);
    fputs("You are awesome! \n",writefp);
    fflush(writefp);

    fclose(writefp);
    fgets(buf, sizeof(buf), readfp);
    fputs(buf,stdout);
    fclose(readfp);

    close(serv_sock);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
```



sep_clnt.c

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void errorHandling(const char *message);

int main(int argc, char *argv[])
{
    int sock;
    char buf[BUF_SIZE] = {0,};
    int str_len;
    struct sockaddr_in serv_adr;

    FILE *readfp;
    FILE *writefp;

    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        errorHandling("socket() error");
    }

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("connect() error!");
    }
    else
    {
        puts("Connected......");
    }

    readfp = fdopen(sock, "r");
    writefp = fdopen(sock, "w");
    while (1)
    {
        if(fgets(buf,sizeof(buf),readfp)==NULL)
            break;
        fputs(buf,stdout);
        fflush(stdout);
    }

    fputs("From CLIENT: Thank you! \n",writefp);
    fflush(writefp);
    fclose(writefp);
    fclose(readfp);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
```



编译运行

```shell
$ gcc sep_serv.c -o ./bin/sep_serv
$ ./bin/sep_serv 9190
```



```shell
$ gcc sep_clnt.c -o ./bin/sep_clnt
$ ./bin/sep_clnt 127.0.0.1 9190
Connected......
From server: Hi,client? 
I love all of the world 
You are awesome! 
```



可与看到服务器端关闭输出模式FILE指针后，并没有再收到客户端发送的最后一条信息。服务器端执行fclose(writefp)后，已经彻底关闭套接字，当然无法再收到信息。



## 3、文件描述符的复制和半关闭

### 3.1 终止流时无法半关闭的原因

sep_serv.c示例中的2个FILE指针，文件描述及套接字之间的关系如下：

<img src="https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-16%20%E4%B8%8B%E5%8D%886.42.54.png" alt="截屏2021-05-16 下午6.42.54" style="zoom: 67%;" />



由上图可知，示例sep_serv.c中读模式FILE指针和写模式FILE指针都是基于同一个文件描述符。因此对任意一个FILE指针调用fclose函数都会关闭文件描述符。

<img src="https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-16%20%E4%B8%8B%E5%8D%886.44.45.png" alt="截屏2021-05-16 下午6.44.45" style="zoom: 67%;" />



那如何进入可以输入但无法输出的半关闭状态呢？创建FILE指针前现复制文件描述符即可。

<img src="https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-16%20%E4%B8%8B%E5%8D%886.46.34.png" alt="截屏2021-05-16 下午6.46.34" style="zoom: 67%;" />



这时，针对写模式FILE指针调用fclose函数时，只能销毁与该FILE指针相关的文件描述符，无法销毁套接字。

<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-05-16 下午6.49.23.png" alt="截屏2021-05-16 下午6.49.23" style="zoom: 67%;" />



如上图所示：掉哟过fclose函数后还剩1个文件描述符，因此没有销毁套接字。但此时还没有进入半关闭状态，只是准备好了半关闭环境。



### 3.2 复制文件描述符

在同一个进程内完成文件描述符的复制，如下图所示：

<img src="https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-16%20%E4%B8%8B%E5%8D%886.52.32.png" alt="截屏2021-05-16 下午6.52.32" style="zoom: 67%;" />



图16-5 给出的是同一进程内存在2个文件描述符可以同时访问文件的情况。当然，文件描述符的值不能重复。



dup & dup2

下面给出文件描述符的复制方法，通过下列2个函数之一完成。

```c
#include <unistd.h>

int dup(int fildes);
int dup2(int fildes, int fildes2);

// 成功时返回复制的文件描述符，失败时返回-1

/*
	fildes 需要复制的文件描述符
	fildes 明确指定的文件描述符整数值
*/
```

下面编写程序验证dup和dup2的功能：

```c
#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]){
    int cfd1,cfd2;
    char str1[] = "Hi ~\n";
    char str2[] = "Nice to meet you! \n";

    cfd1 = dup(1);
    cfd2 = dup2(cfd1, 7);
    printf("fd1=%d, fd2=%d \n",cfd1, cfd2);
    write(cfd1, str1, sizeof(str1));
    write(cfd2, str2, sizeof(str2));

    close(cfd1);
    close(cfd2);

    write(1, str1, sizeof(str1));
    close(1);
    write(1, str2, sizeof(str2));
    return 0;
}
```



编译运行

```shell
$ gcc dup.c -o ./bin/dup
$ ./bin/dup
fd1=3, fd2=7 
Hi ~
Nice to meet you! 
Hi ~
```



### 3.3 复制文件描述符后“流”的分离

下面更改sep_serv.c，使其能正常工作。正常工作是指通过服务器端的半关闭状态接收客户端最后发送的字符串。

sep_serv2.c

```c
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024
void errorHandling(const char *message);

int main(int argc, char *argv[])
{
    int str_len;
    int serv_sock, clnt_sock;
    char buf[BUF_SIZE] = {
        0,
    };

    FILE *readfp;
    FILE *writefp;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    if (argc != 2)
    {
        printf("Usage: %s <port> \n", argv[1]);
        exit(1);
    }

    //调用socket函数创建套接字
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        errorHandling("socket() error");
    }
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    //调用bind函数分配IP地址和端口号
    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("bind() error!");
    }

    //调用listen函数将套接字转化为可接收连接状态
    if (listen(serv_sock, 5) == -1)
    {
        errorHandling("listen() error!");
    }
    clnt_adr_sz = sizeof(clnt_adr);

    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);

    readfp = fdopen(clnt_sock, "r");
    writefp = fdopen(dup(clnt_sock), "w");    // 通过dup函数，复制文件描述符，并将其转换为FILE指针

    fputs("From server: Hi,client? \n", writefp);
    fputs("I love all of the world \n", writefp);
    fputs("You are awesome! \n", writefp);
    fflush(writefp);
    
    shutdown(fileno(writefp), SHUT_WR);   // 调用shutdown函数时， 无论复制出多少文件描述都进入半关闭状态
    fclose(writefp);                      // 关闭写端，此时套接字并没有关闭，还有读模式FILE指针

    fgets(buf, sizeof(buf), readfp);
    fputs(buf, stdout);
    fclose(readfp);                       // 关闭读模式FILE指针，此时才完全关闭套接字

    close(serv_sock);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
```



sep_clnt.c不变



编译运行

```shell
$ gcc sep_serv2.c -o ./bin/sep_serv2$ ./bin/sep_serv2 9190From CLIENT: Thank you! 
```



```shell
$ gcc sep_clnt.c -o ./bin/sep_clnt$ ./bin/sep_clnt 127.0.0.1 9190Connected......From server: Hi,client? I love all of the world You are awesome! 
```



无论复制出多少文件描述，均应调用shutdown函数发送EOF并进入半关闭状态。



4、习题（参考答案）

![截屏2021-05-16 下午7.25.53](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-05-16 下午7.25.53.png)

> （1）
>
> 答：a，文件描述符并不分为输入描述符和输出描述符，一个文件描述符，从一端输入，从另一端输出。
>
> ​		b，复制文件描述符后，将生成新的文件描述符，只是新旧两个文件描述符都指向相同的文件。
>
> ​		c，需要将文件描述符转换为FILE结构体指针才能使用FILE结构体指针进行I/O
>
> ​		e，文件描述符，不区分读和写模式。



> （2）
>
> 答：
>
> a，终止所有文件描述符才能发送EOF
>
> b，此时可能复制了文件描述符，只关闭输出流FILE指针不会发送EOF，需要同时关闭输入流FILE指针或调用shutdown函数才能发送EOF
>
> c，除了关闭文件描述符会发送EOF，调用shutdown函数，无论复制了多少个文件描述符都会发送EOF# 第 17 章 优于select的epoll

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

echo_epollserv.c

```c
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define BUF_SIZE 1024
#define EPOLL_SIZE 50

void errorHandling(const char *message);

int main(int argc, char *argv[])
{
    int str_len, state;
    int serv_sock, clnt_sock;
    char message[BUF_SIZE];
    int fds[2];

    int fd_max, fd_num;
    struct epoll_event *ep_events;
    struct epoll_event event;
    int epfd, event_cnt;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    if (argc != 2)
    {
        printf("Usage: %s <port> \n", argv[1]);
        exit(1);
    }

    //调用socket函数创建套接字
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        errorHandling("socket() error");
    }
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    //调用bind函数分配IP地址和端口号
    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("bind() error!");
    }

    //调用listen函数将套接字转化为可接收连接状态
    if (listen(serv_sock, 5) == -1)
    {
        errorHandling("listen() error!");
    }
    //write函数用于传输数据，若程序经accept函数运行到本行，说明已经有了连接请求
    //调用accept函数受理连接请求，如果在没有连接请求的情况下调用该函数，则不会返回，直到有连接请求为止。

    epfd = epoll_create(EPOLL_SIZE);
    ep_events = malloc(sizeof(struct epoll_event)*EPOLL_SIZE);

    event.events = EPOLLIN;
    event.data.fd = serv_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

    while (1)
    {
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if(event_cnt==-1){
            puts("epoll_wait() error");
            break;
        }

        for(int i=0; i<event_cnt; i++){
            if (ep_events[i].data.fd == serv_sock)
            {
                clnt_adr_sz = sizeof(clnt_adr);
                clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
                event.events=EPOLLIN;
                event.data.fd = clnt_sock;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
                printf("Connect client: %d\n", clnt_sock);
            }else{
                str_len = read(ep_events[i].data.fd, message, BUF_SIZE);
                if (str_len == 0)
                {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
                    close(ep_events[i].data.fd);
                    printf("close client :%d \n", i);
                }
                else
                {
                    write(ep_events[i].data.fd, message, str_len);
                }
            }
        }

    }

    close(serv_sock);
    close(epfd);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
```



[echo_mpclient.c](https://github.com/wangjunstf/computer-system/blob/main/%E8%AE%A1%E7%AE%97%E6%9C%BA%E7%BD%91%E7%BB%9C%E5%8E%9F%E7%90%86/TCP-IP-%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B/ch12/src/echo_mpclient.c)



编译运行

```shell
$ gcc echo_epollserv.c -o ./bin/echo_epollserv
$ ./bin/echo_epollserv 9190
Connect client: 5
close client :0 
```



```shell
$ gcc echo_mpclient.c -o ./bin/echo_mpclient
$ ./bin/echo_mpclient 127.0.0.1 9190
Connected......
123
Message from server: 123
hello world
Message from server: hello world
how are you?
Message from server: how are you?
^C
```



## 3、条件触发

**条件触发**

例如：服务器端输入缓冲收到50字节的数据，服务器端操作系统将通知该事件（注册到发生变化的文件描述符）但服务器端读取20字节后还剩30字节的情况下，仍会注册事件。

只要输入缓冲中还剩有数据，就将以事件方式再次注册。

**边缘触发**

输入缓冲收到数据时仅注册1次该事件。即使输入缓冲中还留有数据，也不会再进行注册。

下面通过修改echo_epollserv.c，来验证条件触发的事件注册方式。

### 实现条件触发服务器端

echo_EPLTserv.c

```c
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define BUF_SIZE 4
#define EPOLL_SIZE 50

void errorHandling(const char *message);

int main(int argc, char *argv[])
{
    int str_len, state;
    int serv_sock, clnt_sock;
    char message[BUF_SIZE];
    int fds[2];

    int fd_max, fd_num;
    struct epoll_event *ep_events;
    struct epoll_event event;
    int epfd, event_cnt;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    if (argc != 2)
    {
        printf("Usage: %s <port> \n", argv[1]);
        exit(1);
    }

    //调用socket函数创建套接字
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        errorHandling("socket() error");
    }
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    //调用bind函数分配IP地址和端口号
    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("bind() error!");
    }

    //调用listen函数将套接字转化为可接收连接状态
    if (listen(serv_sock, 5) == -1)
    {
        errorHandling("listen() error!");
    }
    //write函数用于传输数据，若程序经accept函数运行到本行，说明已经有了连接请求
    //调用accept函数受理连接请求，如果在没有连接请求的情况下调用该函数，则不会返回，直到有连接请求为止。

    epfd = epoll_create(EPOLL_SIZE);
    ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

    event.events = EPOLLIN;
    event.data.fd = serv_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

    while (1)
    {
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if (event_cnt == -1)
        {
            puts("epoll_wait() error");
            break;
        }
        puts("return epoll_wait");
        for (int i = 0; i < event_cnt; i++)
        {
            if (ep_events[i].data.fd == serv_sock)
            {
                clnt_adr_sz = sizeof(clnt_adr);
                clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
                event.events = EPOLLIN;
                event.data.fd = clnt_sock;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
                printf("Connect client: %d\n", clnt_sock);
            }
            else
            {
                str_len = read(ep_events[i].data.fd, message, BUF_SIZE);
                if (str_len == 0)
                {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
                    close(ep_events[i].data.fd);
                    printf("close client :%d \n", i);
                }
                else
                {
                    write(ep_events[i].data.fd, message, str_len);
                }
            }
        }
    }

    close(serv_sock);
    close(epfd);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
```



[echo_mpclient.c](https://github.com/wangjunstf/computer-system/blob/main/%E8%AE%A1%E7%AE%97%E6%9C%BA%E7%BD%91%E7%BB%9C%E5%8E%9F%E7%90%86/TCP-IP-%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B/ch12/src/echo_mpclient.c)



编译运行

```shell
$ ./bin/echo_EPLTserv 9190
return epoll_wait
Connect client: 5
return epoll_wait
return epoll_wait
return epoll_wait
return epoll_wait
return epoll_wait
return epoll_wait
return epoll_wait
```



```shell
$ gcc echo_mpclient.c -o ./bin/echo_mpclient 
$ ./bin/echo_mpclient 127.0.0.1 9190
Connected......
hello world
Message from server: hello world
how are you!
Message from server: how are you!
```

从运行结果可以看出，每当收到客户端数据时，都会注册该事件，并因此多次调用epoll_wait函数。



可以将上述服务器端echo_EPLTserv.c改成边缘触发，可以将event.events = EPOLLIN;改为event.events = EPOLLIN|EPOLLET;

这样，从客户端接收数据时，仅输出1次"return epoll_wait"字符串，这意味着仅注册1次事件。



测试 echo_EPLTserv2.c

```c
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define BUF_SIZE 4
#define EPOLL_SIZE 50

void errorHandling(const char *message);

// 用于测试边缘触发
int main(int argc, char *argv[])
{
    int str_len, state;
    int serv_sock, clnt_sock;
    char message[BUF_SIZE];
    int fds[2];

    int fd_max, fd_num;
    struct epoll_event *ep_events;
    struct epoll_event event;
    int epfd, event_cnt;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    if (argc != 2)
    {
        printf("Usage: %s <port> \n", argv[1]);
        exit(1);
    }

    //调用socket函数创建套接字
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        errorHandling("socket() error");
    }
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    //调用bind函数分配IP地址和端口号
    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("bind() error!");
    }

    //调用listen函数将套接字转化为可接收连接状态
    if (listen(serv_sock, 5) == -1)
    {
        errorHandling("listen() error!");
    }
    //write函数用于传输数据，若程序经accept函数运行到本行，说明已经有了连接请求
    //调用accept函数受理连接请求，如果在没有连接请求的情况下调用该函数，则不会返回，直到有连接请求为止。

    epfd = epoll_create(EPOLL_SIZE);
    ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

    event.events = EPOLLIN|EPOLLET;
    event.data.fd = serv_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

    while (1)
    {
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if (event_cnt == -1)
        {
            puts("epoll_wait() error");
            break;
        }
        puts("return epoll_wait");
        for (int i = 0; i < event_cnt; i++)
        {
            if (ep_events[i].data.fd == serv_sock)
            {
                clnt_adr_sz = sizeof(clnt_adr);
                clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
                event.events = EPOLLIN;
                event.data.fd = clnt_sock;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
                printf("Connect client: %d\n", clnt_sock);
            }
            else
            {
                str_len = read(ep_events[i].data.fd, message, BUF_SIZE);
                if (str_len == 0)
                {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
                    close(ep_events[i].data.fd);
                    printf("close client :%d \n", i);
                }
                else
                {
                    write(ep_events[i].data.fd, message, str_len);
                }
            }
        }
    }

    close(serv_sock);
    close(epfd);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
```



echo_client.c

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void errorHandling(const char *message);

int main(int argc, char *argv[])
{
    int sock;
    char message[BUF_SIZE];
    int str_len;
    struct sockaddr_in serv_adr;

    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        errorHandling("socket() error");
    }

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("connect() error!");
    }
    else
    {
        puts("Connected......");
    }

    while (1)
    {
        fputs("Input message(Q to quit):", stdout);
        fgets(message, BUF_SIZE, stdin);
        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
            break;

        write(sock, message, strlen(message));
        str_len = read(sock, message, BUF_SIZE - 1);
        message[str_len] = 0;
        printf("Message from server: %s", message);
    }

    close(sock);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
```



编译运行

```shell
$ gcc echo_EPLTserv2.c -o ./bin/echo_EPLTserv2
$ ./bin/echo_EPLTserv2 9190
return epoll_wait
Connect client: 5
return epoll_wait
return epoll_wait
return epoll_wait
```



```shell
$ gcc echo_client.c -o ./bin/echo_client
$ ./bin/echo_client 127.0.0.1 9190
Connected......
Input message(Q to quit):hello world
Message from server: hello world
Input message(Q to quit):
```



可能是版本的原因，此处输出了多次"return epoll_wait"字符串，类似于条件触发。



## 4、边缘触发

边缘触发必知两点内容：

* 通过errno变量验证错误原因。
* 为了完成非阻塞(Non-blocking)I/O，更改套接字特性

为了在发生错误时提供额外的信息，Linux声明了如下全局变量 int errno;

为了引入该变量，需引入头文件error.h头文件，因为此头文件中有上述变量的extern声明。

每种函数发生错误时，保存到errno变量的值都不相同，在需要时查阅即可：

“read函数发现输入缓冲中没有数据可读时返回-1，同时在errno中保存EAGAIN常量”



Linux 提供更改或读取文件属性的如下方法



```c
#include <fcntl.h>

int fcntl(int filedes, int cmd, ...);
// 成功时返回cmd参数相关值，失败时返回-1
/*
	filedes 需要被读取或设置属性的文件描述符
	cmd 表示函数调用目的
*/
```



从上述声明中可以看到，fcntl具有可变参数的形式，如果向第二个参数传递F_GETFL，可以获得第一个参数所指的文件描述符属性

如果向第二个参数传递F_SETFL，可以更改文件描述符属性

例如：将文件（套接字）改为非阻塞模式，需要如下两条语句：

```c
int flag = fcntl(fd, F_GETFL, 0);          // 获取之前设置的属性信息
fcntl(fd,F_SETFL, flag|O_NONBLOCK);        // 在原有属性信息不变的情况下，添加非阻塞标志
```

调用read & write函数，无论是否存在数据，都会形成非阻塞文件(套接字)



### 实现边缘触发的服务器端

首先说明为何需要errno确认错误原因

“边缘触发方式中，接收数据时仅注册1次该事件”

因为这种特点，一旦发生输入相关事件，就应该立即读取输入缓冲中的全部数据。因此需要验证输入缓冲是否为空。

“read函数返回-1，变量errno中的值为EAGAIN时，说明没有数据可读”

既然如此，为何还需要将套接字变成非阻塞模式？变缘触发方式下，以阻塞方式工作的read & write函数有可能引起服务器端的长时间停顿。因此，边缘触发方式中一定要采用非阻塞read & write函数。

接下来给出边缘触发方式工作的回声服务器端示例。

echo_EPETserv.c

```c
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZE 4
#define EPOLL_SIZE 50

void errorHandling(const char *message);
void setnonblockingmode(int fd) ;
int main(int argc, char *argv[])
{
    int str_len, state;
    int serv_sock, clnt_sock;
    char message[BUF_SIZE];
    int fds[2];

    int fd_max, fd_num;
    struct epoll_event *ep_events;
    struct epoll_event event;
    int epfd, event_cnt;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    if (argc != 2)
    {
        printf("Usage: %s <port> \n", argv[1]);
        exit(1);
    }

    //调用socket函数创建套接字
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        errorHandling("socket() error");
    }
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    //调用bind函数分配IP地址和端口号
    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("bind() error!");
    }

    //调用listen函数将套接字转化为可接收连接状态
    if (listen(serv_sock, 5) == -1)
    {
        errorHandling("listen() error!");
    }
    //write函数用于传输数据，若程序经accept函数运行到本行，说明已经有了连接请求
    //调用accept函数受理连接请求，如果在没有连接请求的情况下调用该函数，则不会返回，直到有连接请求为止。

    epfd = epoll_create(EPOLL_SIZE);
    ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);
    setnonblockingmode(serv_sock);
    event.events = EPOLLIN;
    event.data.fd = serv_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

    while (1)
    {
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if (event_cnt == -1)
        {
            puts("epoll_wait() error");
            break;
        }
        puts("return epoll_wait");
        for (int i = 0; i < event_cnt; i++)
        {
            if (ep_events[i].data.fd == serv_sock)
            {
                clnt_adr_sz = sizeof(clnt_adr);
                clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
                setnonblockingmode(clnt_sock);
                event.events = EPOLLIN|EPOLLET;
                event.data.fd = clnt_sock;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
                printf("Connect client: %d\n", clnt_sock);
            }
            else
            {
                while(1){
                    str_len = read(ep_events[i].data.fd, message, BUF_SIZE);
                    if (str_len == 0)
                    {
                        epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
                        close(ep_events[i].data.fd);
                        printf("close client :%d \n", i);
                        break;
                    }else if(str_len<0){
                        if(errno==EAGAIN)
                            break;
                        
                    }else{
                        write(ep_events[i].data.fd, message, str_len);
                    }
                }
            }
        }
    }

    close(serv_sock);
    close(epfd);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void setnonblockingmode(int fd){
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag|O_NONBLOCK);
}
```



echo_mpclient.c

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void errorHandling(const char *message);
void read_routine(int sock, char *buf);
void write_routine(int sock, char *buf);
int itoc(int num, char *str);

int main(int argc, char *argv[])
{
    int sock;
    char message[BUF_SIZE];
    int str_len;
    struct sockaddr_in serv_adr;

    pid_t pid;

    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        errorHandling("socket() error");
    }

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("connect() error!");
    }
    else
    {
        puts("Connected......");
    }

    pid = fork();
    if (pid == 0)
    {
        write_routine(sock, message);
    }
    else
    {
        read_routine(sock, message);
    }
    close(sock);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void read_routine(int sock, char *buf)
{
    while (1)
    {
        int str_len = read(sock, buf, BUF_SIZE);
        if (str_len == 0)
            return;

        buf[str_len] = 0;
        printf("Message from server: %s", buf);
    }
}

void write_routine(int sock, char *buf)
{
    while (1)
    {
        fgets(buf, BUF_SIZE, stdin);
        if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n"))
        {
            shutdown(sock, SHUT_WR);
            return;
        }

        write(sock, buf, strlen(buf));
    }
}

int itoc(int num, char *str)
{
    char tem[1024];
    int id = 0, id2 = 0;
    while (num)
    {
        int t = num % 10;
        tem[id++] = t + '0';
        num /= 10;
    }
    str[id--] = '\0';
    while (id >= 0)
    {
        str[id2++] = tem[id--];
    }
    return 0;
}
```



编译运行

```shell
$ gcc echo_EPETserv.c -o ./bin/echo_EPETserv
$ ./bin/echo_EPETserv 9190
return epoll_wait
Connect client: 5
return epoll_wait
return epoll_wait
```

```
$ gcc echo_mpclient.c -o ./bin/echo_mpclient
$ ./bin/echo_mpclient 127.0.0.1 9190
Connected......
hello world
Message from server: hello world
I love computer programming
Message from server: I love computer programming
```

可以看到，客户端每发送一次数据，服务器端也相应产生几次事件。



## 5、条件触发和边缘触发的优劣

边缘触发可以做到如下这点：

“可以分离接收数据和处理数据的时间点。”



即使输入缓冲收到数据（注册相应事件），服务器端也能决定读取和处理这些数据的时间点，这样就能给服务器端的实现带来巨大的灵活性



条件触发和边缘触发的区别主要应该从服务器端实现模型的角度讨论。



从实现模型的角度看，边缘触发更有可能带来高性能。



## 7、习题（参考答案）

<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-05-19 上午12.06.56.png" alt="截屏2021-05-19 上午12.06.56"  />

（1）

* 调用select函数后常见的针对所有文件描述符循环语句
* 每次调用select函数时都需要向该函数传递监视对象信息



（2）

因为套接字是由操作系统管理的，所以无论是select还是epoll都需要将监视对象文件描述符信息通过函数传递给操作系统。



（3）

差异是每次调用select函数时都需要向该函数传递监视对象信息，而epoll只需传递一次。

有些函数必须借助于操作系统，select函数与文件描述符有关，更准确地说，是监视套接字变化的函数，而套接字是由操作系统管理的，所以select函数需要借助于操作系统才能完成。



（4）

* 服务器端接入者少
* 程序应具有兼容性



（5）

二者的区别主要在于服务器端实现模型。条件触发方式中，只要输入缓冲中有数据，就会一直通知该事件，而边缘触发，输入缓冲收到数据时仅注册一次该事件，即使输入缓冲中还留有数据，也不会再进行注册。



（6）

原因：输入缓冲收到数据时仅注册一次该事件，即使输入缓冲中还留有数据，也不会再进行注册。

优点：给服务器端的实现带来巨大的灵活性。


# 第 18 章 多线程服务器端的实现

## 1、理解线程的概念

### 1.1 引入线程的概念

如前所述，创建进程的工作本身会给操作系统带来相当沉重的负担。而且每个进程具有独立的内存空间，所以进程间通信的实现难度也会随之提高。换言之，多进程模型的缺点可概括如下。

* 创建进程的过程会带来一定的开销
* 为了完成进程间数据交换，需要特殊的IPC技术

相比上述两点，“每秒少则数10次，多则数千次的‘上下文切换(Context Switching)’是创建进程时最大的开销。”



由于需要运行的进程数远远多于CPU核心数，为了提高CPU运行效率，采用了事件片轮转的方式实现了宏观上的并发执行。即每个进程独占一个CPU核心运行一定时间后，切换为下一个进程执行，由于每个进程执行的时间片极短，因此宏观上感觉所有进程在并发执行。但频繁的进程切换也需要产生较大的开销。



为了保持多进程的优点，同时在一定程度上克服其缺点，人们引入了线程"Thread"。这是一种将进程的各种劣势降至最低限度（不是直接消除）而设计的一种“轻量级进程”。线程相比于进程具有如下优点：

* 线程的创建和上下文切换比进程的创建和上下文切换更快
* 线程间交换数据时无需特殊技术



### 1.2 线程和进程的差异

每个进程的内存空间都由保存全局变量的“数据区”，向malloc等函数的动态分配提供空间的堆(Heap)，函数运行时使用的栈(Stack)构成。每个进程都拥有这种独立的空间，多个进程的内存结构如下图所示。

<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-05-19 下午8.41.10.png" alt="截屏2021-05-19 下午8.41.10" style="zoom: 67%;" />


而如果获得多个代码执行流为主要目的，则不应该向图18-1那样完全分离内存结构，而只需分离栈区域，通过这种方式可以获得如下优势：

* 上下文切换时不需要切换数据区和堆。
* 可以利用数据区和堆交换数据。

线程的内存结构如下图所示：

<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-05-19 下午8.44.48.png" alt="截屏2021-05-19 下午8.44.48" style="zoom:67%;" />



进程和线程可以定义为如下形式：

* 进程：在操作系统构成单独执行流的单位。
* 线程：在进程构成单独执行流的单位。

操作系统，进程，线程之间的关系如下图所示：

<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-05-19 下午8.50.06.png" alt="截屏2021-05-19 下午8.50.06" style="zoom: 67%;" />



## 2、线程创建及运行

线程具有单独的执行流，因此需要单独定义线程的main函数，还需要请求操作系统在单独的执行流中执行该函数，完成该功能的函数如下：

```c
#include <pthread.h>

int pthread_create(
pthread_t* restrict thread, const pthread_attr_t * restrict attr, 
void * (* start_routine)(void *), void * restrict arg);

// 成功时返回0， 失败时返回其他值

/*
	thread 保存新创建线程ID的变量地址值，线程与进程相同，也需要用于区分不同线程的ID。
	arrt 用于传递线程属性的参数，传递NULL时，创建默认属性的线程
	start_routing 相等于线程的main函数，在单独执行流中执行的函数地址值（函数指针）
	arg 通过第三个参数传递的函数的形参变量地址值
*/

```

restrict的作用，确保多个指针不指向同一数据。

通过以下示例了解该函数的功能。

thread1.c

```c
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void * thread_main(void *args);

int main(int argc, char* argv[]){
    pthread_t t_id;
    int thread_param = 5;

    if(pthread_create(&t_id, NULL, thread_main, (void*)&thread_param) !=0){
        puts("pthread_create() error");
        return -1;
    }
    
    sleep(10);
    puts("end of main");

    return 0;
}

void * thread_main(void * args){
    int cnt = *((int*)args);
    for(int i=0; i<cnt; i++){
        sleep(1);
        puts("running thread");
    }
    return NULL;
}
```



编译运行

编译时需要使用 -lpthread来使用线程库

```shell
$ gcc thread1.c -o ./bin/thread1 -lpthread
$ gcc thread1.c -o ./bin/thread1 -lpthread
$ ./bin/thread1 
running thread
running thread
running thread
running thread
running thread
end of main
```



thread1.c的执行流程如下图所示：

<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-05-19 下午9.20.25.png" alt="截屏2021-05-19 下午9.20.25" style="zoom:50%;" />



代码中 sleep(10);是为了给线程运行提供时间，不然main函数结束后，线程也就终止运行了。如果不使用sleep(10)，程序就如下图所示执行：

<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-05-19 下午9.22.04.png" alt="截屏2021-05-19 下午9.22.04" style="zoom:50%;" />



上述调用sleep函数只是为了演示，在实际开发中，通常利用下面的函数控制线程的执行流。

```c
#include <pthread.h>

int pthread_join(pthread_t thread, void **status);
// 成功时返回0， 失败时返回其他值

/*
	thread 该参数值ID的线程终止后才会从该函数返回
	status 保存线程的main函数返回值的指针变量的地址值
*/
```

下面通过示例了解该函数：

thread2.c

```c
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void *thread_main(void *args);

int main(int argc, char *argv[])
{
    pthread_t t_id;
    int thread_param = 5;
    void * thr_ret;

    if (pthread_create(&t_id, NULL, thread_main, (void *)&thread_param) != 0)
    {
        puts("pthread_create() error");
        return -1;
    }

    if(pthread_join(t_id, &thr_ret)!=0){
        puts("pthread_join() error");
        return -1;
    }

    printf("Thread return message: %s",(char*)thr_ret);
    free(thr_ret);
    return 0;
}

void *thread_main(void *args)
{
    int cnt = *((int *)args);
    char *msg = (char*)malloc(sizeof(char)*50);
    strcpy(msg, "Hi, I am thread\n");
    for (int i = 0; i < cnt; i++)
    {
        sleep(1);
        puts("running thread");
    }
    return msg;
}
```



编译运行

```c
$ gcc thread2.c -o ./bin/thread2 -lpthread
$ ./bin/thread2
running thread
running thread
running thread
running thread
running thread
Thread return message: Hi, I am thread
```

该示例的执行流程图如下：

<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-05-19 下午9.35.25.png" alt="截屏2021-05-19 下午9.35.25" style="zoom:50%;" />



## 3、可在临界区调用的函数

关于线程的运行需要考虑“多个线程同时调用函数时（执行时）可能产生问题”。这类函数内部存在临界区(Critical Section)，也就是说，多个线程同时执行这部分代码时，可能引起问题，临界区中至少存在一条这类代码。

根据临界区是否引起问题，函数可分为以下两类：

* 线程安全函数(Thread-safe function)
* 非线程安全函数(Thread-unsafe function)

线程安全函数被多个线程同时调用时也不会引起问题，反之，非线程安全函数同时被调用时可能会引发问题。

Unix或windows在定义非线程安全函数的同时，提供了具有相同功能的线程安全函数。比如第 8 章介绍的如下函数就不是线程安全函数

```c
struct hostent * gethostbyname(const char* hostname);
```

同时提供了线程安全的同一功能的函数：

```c
struct hostent * gethostbyname_r (const char * name, struct hostent * result, char * buffer, int buflen, int *h_errnop);
```



Unix下线程安全函数的名称后缀通常为_r。



可以在编译时通过添加 -D_REENTRANT选项定义宏，可以将非线程安全函数调用给问线程安全函数调用。

```shell
$ gcc -D_REENTRANT thread.c -o ./bin/thread -lpthread
```



## 4、工作（Worker）线程模型

接下来给出多线程示例，计算1到10的和，但并不是在main函数中进行累加运算，而是创建2个线程，其中一个线程计算1到5的和，另一个线程计算6到10的和，main函数只负责输出运算结果。这种模式的编程模型称为“工作线程（Worker thread）”模型。计算1到5之和的线程与计算6到10之和的线程将称为main线程管理的工作(Worker)。以下是该示例的执行流程图：

<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-05-19 下午11.26.28.png" alt="截屏2021-05-19 下午11.26.28" style="zoom:50%;" />



thread3.c

```c
#include <stdio.h>
#include <pthread.h>

void* thread_summation(void* arg);
int sum=0;

int main(int argc, char* argv[]){
    pthread_t id_t1, id_t2;
    int range1[] = {1,5};
    int range2[] = {6,10};

    pthread_create(&id_t1, NULL, thread_summation, (void*)range1);
    pthread_create(&id_t2, NULL, thread_summation, (void*)range2);

    pthread_join(id_t1, NULL);
    pthread_join(id_t2, NULL);
    printf("result : %d\n",sum);
    return 0;
}

void * thread_summation(void *arg){
    int start = ((int*)arg)[0];
    int end = ((int*)arg)[1];
    while(start<=end){
        sum+=start;
        start++;
    }
    return NULL;
}

```



编译运行

```shell

$ gcc -D_REENTRANT thread3.c -o ./bin/thread3 -lpthread
$ ./bin/thread3
result : 55
```



运行结果是55，结果虽然正确，但示例本身存在问题。此处存在临界区相关问题。因此再介绍另一示例，该示例与上述示例相似，只是增加了发生临界区相关错误的可能性。

thread4.c

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREAD 100
void* thread_inc(void* arg);
void* thread_des(void* arg);
long long num=0;

int main(int argc, char* argv[]){
    pthread_t thread_id[NUM_THREAD];
    for(int i=0; i<NUM_THREAD; i++){
        if(i%2){
            pthread_create(&thread_id[i], NULL, thread_inc, NULL);
        }else{
            pthread_create(&thread_id[i], NULL, thread_des, NULL);
        }
    }

    for(int i=0; i<NUM_THREAD; i++){
        pthread_join(thread_id[i], NULL);
    }

    printf("result : %lld\n",num);
    return 0;
}

void * thread_inc(void * arg){
    for(int i=0; i<10000;i++){
        num+=1;
    }

    return NULL;
}

void *thread_des(void *arg)
{
    for (int i = 0; i < 10000; i++)
    {
        num -= 1;
    }

    return NULL;
}
```



编译运行

```shell
$ gcc -D_REENTRANT thread4.c -o ./bin/thread4 -lpthread
$ ./bin/thread4
result : -14490
$ ./bin/thread4
result : 9245
$ ./bin/thread4
result : -16355
$ ./bin/thread4
```



运行结果并不是0，而且每次运行结果都不相同，虽然原因尚不得而知，但可以肯定的是，这对于线程的运用是个大问题。



## 5、线程存在的问题和临界区

### 5.1 多个线程访问同一变量是问题

示例 thread4.c存在的问题如下：

“2个线程正在同时访问全局变量num”

此处的“同时访问”与平时所接触的同时有一定区别，下面通过示例解释“同时访问”的含义，并说明为何会存在问题。



假设2个线程要执行变量逐次加1的工作，如下图所示：

<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-05-20 上午12.03.43.png" alt="截屏2021-05-20 上午12.03.43" style="zoom:50%;" />



图18-8中描述的是2个线程准备将变量num的值加1的情况。在此状态下，线程1将变量num的值增加到100，线程2再访问num时，变量num中将按照我们的预想保存101。下图是线程1将num完全增加后的情形。

<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-05-20 上午12.07.14.png" alt="截屏2021-05-20 上午12.07.14" style="zoom:50%;" />



图18-9中需要注意值的增加方式，值的增加需要CPU运算完成，变量num中的值不会自动增加。线程1首先读该变量的值并将其传递给CPU，获得加1之后的结果100，最后再把结果写回num，这样num中就保存100。接下来线程2的执行过程如图：

<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-05-20 上午12.11.13.png" alt="截屏2021-05-20 上午12.11.13" style="zoom:50%;" />



变量num将保存101，这是最理想的情况。线程1完全增加num值之前，线程2完全有可能通过切换得到cpu资源。如下图所示，线程1读取变量num的值并完成加1运算时的情况，只是加1后的结果尚未写入变量num

<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-05-20 上午12.13.59.png" alt="截屏2021-05-20 上午12.13.59" style="zoom: 67%;" />



接下来就要将100保存到变量num中，但执行该操作前，执行流程跳转到了线程2，线程2完成了加1运算，并将加1之后的结果写入变量num，如下图所示：

<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-05-20 上午12.16.06.png" alt="截屏2021-05-20 上午12.16.06" style="zoom:67%;" />



从图18-12中可以看出，变量num的值尚未被线程1加1到100，因此线程2读到变量num的值为99，结果是线程2将num值改为100，还剩下线程1将运算后的结果写入变量num的操作。接下来给出过程：

<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-05-20 上午12.18.19.png" alt="截屏2021-05-20 上午12.18.19" style="zoom:67%;" />



此时线程1将自己的运算结果100再次写入变量num，结果变量num变成100，虽然线程1和线程2各做了1次加1运算，却得到了意想不到的结果。因此，线程访问变量num时应该阻止其他线程访问，直到线程1完成运算。这就是同步(Synchronization)。



### 5.2 临界区位置

临界区的定义：

“函数内同时运行多个线程时引起问题的多条语句构成的代码块”



全局变量num是否是临界区？

不是，因为它不是引起问题的语句。该变量并非同时运行，只是代表内存区域的声明而已。临界区通常位于线程运行的函数内部。下面观察thread4.c中两个函数：

```c
void * thread_inc(void * arg){
    for(int i=0; i<10000;i++){
        num+=1;     // 临界区
    }

    return NULL;
}

void *thread_des(void *arg)
{
    for (int i = 0; i < 10000; i++)
    {
        num -= 1;     // 临界区
    }

    return NULL;
}
```



由代码可知，临界区并非num本身，而是访问num的两条语句。这2条语句可能由多个线程同时运行，这也是引起问题的直接原因。产生的问题可以整理为如下3种情况：

* 2个线程同时执行thread_inc函数
* 2个线程同时执行thread_des函数
* 2个线程同时执行thread_inc函数和thread_des函数

“线程1执行thread_inc函数的num+=1语句的同时，线程2执行thread_des函数的num-=1语句”

也就是说：2条不同语句由不同线程同时执行时，也有可能构成临界区。前提是这2条语句访问同一内存区域。



## 6、线程同步

线程同步用于解决线程访问顺序引发的问题。需要同步的情况可以从如下两方面考虑。

* 同时访问同一内存空间时发生的情况。
* 需要指定访问同一内存空间的线程执行顺序的情况。

线程同步采用的常用技术为："互斥量"和“信号量”，二者概念上十分接近。



### 6.1 互斥量

互斥量是"Mutual Exclusion"的简写，表示不允许多个线程同时访问。互斥量主要用于解决线程同步访问的问题。

互斥量就像一把锁，当一个线程使用某个变量时，就把该变量锁住，直到该线程使用完该变量解锁后，其他线程才能访问该变量。

接下来介绍互斥量的创建和销毁函数。

```c
#include <pthread.h>

int pthread_mutex_init(pthread_mutex_t * mutex, const pthread_mutexattr_t* attr);
int pthread_mutex_destroy(pthread_mutex_t * mutex);

// 成功时返回0，失败时返回其他值

/*
	mutex 创建互斥量时传递保存互斥量的变量地址值，销毁时传递需要销毁的互斥量地址值
	attr 传递即将创建的互斥量属性，没有特别需要指定的属性时传递NULL
*/
```



为了创建相当于锁系统的互斥量，需要声明如下pthread_mutex_t型变量：

pthread_mutex_t mutex;



接下来介绍利用互斥量锁住和释放临界区时使用的函数。

```c
#include <pthread.h>

int pthread_mutex_lock(pthread_mutext_t *mutex);
int pthread_mutex_unlock(pthread_mutext_t *mutex);
// 成功时返回0，失败时返回其他值
```



可以通过如下结构保护临界区：

```c
pthread_mutex_lock(&mutex);
//临界区开始
//...
//临界区结束
pthread_mutex_unlock(&mutex);
```

互斥量就像一把锁，阻止多个进程访问临界区。



### 6.2 死锁

当线程退出临界区时，如果忘了调用pthread_mutex_unlock函数，那么其他为了进程临界区而调用pthread_mutex_lock函数的线程就无法摆脱阻塞状态。这样的情况称为“死锁”(Dead-lock)。



接下来利用互斥量解决示例thread4.c中遇到的问题：

mutex.c

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREAD 100
void *thread_inc(void *arg);
void *thread_des(void *arg);
long long num = 0;

pthread_mutex_t mutex;

int main(int argc, char *argv[])
{
    pthread_t thread_id[NUM_THREAD];
    pthread_mutex_init(&mutex, NULL);
    for (int i = 0; i < NUM_THREAD; i++)
    {
        if (i % 2)
        {
            pthread_create(&thread_id[i], NULL, thread_inc, NULL);
        }
        else
        {
            pthread_create(&thread_id[i], NULL, thread_des, NULL);
        }
    }

    for (int i = 0; i < NUM_THREAD; i++)
    {
        pthread_join(thread_id[i], NULL);
    }

    printf("result : %lld\n", num);
    return 0;
}

void *thread_inc(void *arg)
{
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < 1000000; i++)
    {
        num += 1;
    }
    pthread_mutex_unlock(&mutex);

    return NULL;
}

void *thread_des(void *arg)
{
    for (int i = 0; i < 1000000; i++)
    {
        pthread_mutex_lock(&mutex);
        num -= 1;
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}
```



编译运行

```shell
$ gcc mutex.c -D_REENTRANT  -o ./bin/mutex -lpthread
$ ./bin/mutex 
result : 0
$ ./bin/mutex 
result : 0
$ ./bin/mutex 
result : 0
```



从运行结果看，已经解决了thread4.c中的问题。但确认运行时间需要等待较长时间。因为互斥lock，unlock函数的调用过程比想象中花费更长时间。



观察下列代码：

```c
void *thread_inc(void *arg)
{
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < 1000000; i++)
    {
        num += 1;
    }
    pthread_mutex_unlock(&mutex);

    return NULL;
}
```



以上临界区划分范围较大，但这考虑到如下优点所做的决定：

“最大限度减少互斥量lock，unlock函数的调用次数”



缺点就是变量num值怎加到1000000前，都不允许其他线程访问。



到底应该扩大临界区还是缩小临界区，没有标准答案，需要根据实际情况考虑。



### 6.3 信号量

信号量与互斥量极为相似，在互斥量的基础上很容易理解信号量。此处只涉及利用“二进制信号量”（只用0和1）完成“控制线程顺序”为中心的同步方法。

下面给出信号了创建及销毁方法

```c
#include <semaphore.h>

int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_destroy(sem_t * sem);

// 成功时返回0， 失败时返回其他值

/*
	sem 创建信号量时传递的保存信号量的变量地址值，销毁时传递需要销毁的信号量变量地址值
	pshared 传递其他值时，创建可由多个进程共享的信号量； 传递0时，创建只允许1个进程内部使用的信号量。我们需要完成同一进程内的线程同步，故传递0
	value 指定新创建 的信号量初始值
*/
```



接下来介绍相当于互斥量lock，unlock的函数：

```c
#include <semaphore.h>

int sem_post(sem_t * sem);
int sem_wait(sem_t * sem);

//成功时返回0，失败时返回其他

/*
	传递保存信号量读取值的变量地址值，传递给sem_post时信号量增1，传递给sem_wait时信号量减1
*/
```



信号量的值不能小于0，因此，在信号量为0的情况下调用sem_wait函数时，调用函数的线程将进入阻塞状态。此时如果有其他线程调用sem_post函数，信号量的值将变为1，原本阻塞的线程将该信号量重新减为0并跳出阻塞状态。

可以通过如下结构同步临界区，假设信号量的初始值为1

sem_wait(&sem);   //信号量变为0

//临界区开始

//。。。

//临界区的结束

sem_post(&sem); //信号量变为1



调用sem_wait函数进入临界区的线程在调用sem_post函数之前不允许其他线程进入临界区。

现用信号量实现以下功能：

“线程A从用户输入的到值后存入全局变量num，此时线程B将取走该值并累加。该过程进行5次，完成后输出总和并退出程序”

semaphore.c

```c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

void *read(void *arg);
void *accu(void *arg);

static sem_t sem_one;
static sem_t sem_two;
static int num;

int main(int argc, char* argv[]){
    pthread_t id_t1, id_t2;
    sem_init(&sem_one,0,0);
    sem_init(&sem_two,0,1);

    pthread_create(&id_t1, NULL, read, NULL);
    pthread_create(&id_t2, NULL, accu, NULL);

    pthread_join(id_t1, NULL);
    pthread_join(id_t2, NULL);

    sem_destroy(&sem_one);
    sem_destroy(&sem_two);

    return 0;
}

void *read(void *arg){
    for(int i=0; i<5; i++){
        fputs("Input num: ", stdout);
        sem_wait(&sem_two);
        scanf("%d",&num);
        sem_post(&sem_one);
    }
    return NULL;
}

void *accu(void *arg){
    int sum=0;
    for(int i=0; i<5; i++){
        sem_wait(&sem_one);
        sum+=num;
        sem_post(&sem_two);
    }
    printf("sum = %d\n",sum);
    return NULL;
}
```



编译运行

```shell
$ gcc semaphore.c -D_REENTRANT -o ./bin/semaphore -lpthread
$ ./bin/semaphore 
Input num: 1
Input num: 2
Input num: 3
Input num: 4
Input num: 5
sum = 15
```



## 7、线程的销毁和多线程并发服务器端的实现

### 7.1 销毁线程的3种方法

Linux线程并不是在首次调用的线程main函数返回时自动销毁，所以用如下2种方式之一加以明确。否则由线程创建的内存空间将一直存在。

* 调用pthread_join函数
* 调用pthread_detach函数

之前使用pthread_join函数，不仅会等待线程终止，还会引导线程销毁。但该函数的问题是，线程终止前，调用该函数的线程将进入阻塞状态。

为了解决上述问题，通常使用下面的方式来引导线程销毁：

```c
#include <pthread>

int pthread_detach(pthread_t thread);
// 成功时返回0，失败时返回其他值

/*
	thread 终止的同时需要销毁的线程ID
*/
```



调用上述函数不会引起线程终止或进入阻塞状态，可以通过该函数引导销毁销毁线程创建的内存空间。



### 7.2 多线程服务器端的实现

接下来使用多线程实现以下功能：“多个客户端之间可以交换信息的简单的聊天程序”

chat_server.c

```c
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

void errorHandling(const char *message);
void *handle_clnt(void *arg);
void *send_msg(char* msg, int len);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutex;

int main(int argc, char *argv[])
{
    int str_len, state;
    int serv_sock, clnt_sock;
    char message[BUF_SIZE];
    pthread_t t_id;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    if (argc != 2)
    {
        printf("Usage: %s <port> \n", argv[1]);
        exit(1);
    }

    pthread_mutex_init(&mutex, NULL);

    //调用socket函数创建套接字
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        errorHandling("socket() error");
    }
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    //调用bind函数分配IP地址和端口号
    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("bind() error!");
    }

    //调用listen函数将套接字转化为可接收连接状态
    if (listen(serv_sock, 5) == -1)
    {
        errorHandling("listen() error!");
    }
    //write函数用于传输数据，若程序经accept函数运行到本行，说明已经有了连接请求
    //调用accept函数受理连接请求，如果在没有连接请求的情况下调用该函数，则不会返回，直到有连接请求为止。

    
    while (1)
    {
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
        pthread_mutex_lock(&mutex);
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutex);

        pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
        pthread_detach(t_id);
        printf("Connect client ID: %s \n", inet_ntoa(clnt_adr.sin_addr));
    }
    close(serv_sock);
    pthread_mutex_destroy(&mutex);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void * handle_clnt(void * arg){
    int clnt_sock = *((int*)arg);
    int str_len = 0;
    char msg[BUF_SIZE];
    while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0){
        send_msg(msg, str_len);
    }

    pthread_mutex_lock(&mutex);
    for(int i=0; i<clnt_cnt; i++){
        if(clnt_sock==clnt_socks[i]){
            while(i++<clnt_cnt-1){
                clnt_socks[i] = clnt_socks[i+1];
            }
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutex);
    close(clnt_sock);
    return NULL;
}

void* send_msg(char * msg, int len){
    pthread_mutex_lock(&mutex);
    for(int i=0 ; i<clnt_cnt; i++){
        write(clnt_socks[i], msg, len);
    }
    pthread_mutex_unlock(&mutex);
}
```





chat_clnt.c

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

void errorHandling(const char *message);
void* send_msg(void* msg);
void *recv_msg(void* msg);

char name[NAME_SIZE] = "[default]";
char msg[BUF_SIZE];
int main(int argc, char *argv[])
{
    int sock;
    char message[BUF_SIZE];
    int str_len;
    struct sockaddr_in serv_adr;
    pthread_t snd_thread, rcv_thread;
    void *thread_return;

    if (argc != 4)
    {
        printf("Usage : %s <IP> <port> <name>\n", argv[0]);
        exit(1);
    }

    sprintf(name, "[%s]",argv[3]);
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        errorHandling("socket() error");
    }

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("connect() error!");
    }
    else
    {
        puts("Connected......");
    }

    pthread_create(&snd_thread, NULL, send_msg, (void*)&sock); 
    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);

    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);
    close(sock);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void * send_msg(void *arg){
    int sock = *((int*)arg);
    char name_msg[NAME_SIZE+BUF_SIZE];
    while(1){
        fgets(msg, BUF_SIZE, stdin);
        if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")){
            close(sock);
            exit(0);
        }
        sprintf(name_msg, "%s %s",name, msg);
        write(sock, name_msg, strlen(name_msg));
    }
    return NULL;
}

void * recv_msg(void * arg){
    int sock = *((int*)arg);
    char name_msg[NAME_SIZE+BUF_SIZE];
    int str_len;
    while(1){
        str_len = read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
        if(str_len==-1){
            return (void*)-1;  //将-1转换为void指针
        }
        name_msg[str_len]=0;

        fputs(name_msg, stdout);
    }
    return NULL;
}





void read_routine(int sock, char *buf)
{
    while (1)
    {
        int str_len = read(sock, buf, BUF_SIZE);
        if (str_len == 0)
            return;

        buf[str_len] = 0;
        printf("Message from server: %s", buf);
    }
}

void write_routine(int sock, char *buf)
{
    while (1)
    {
        fgets(buf, BUF_SIZE, stdin);
        if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n"))
        {
            shutdown(sock, SHUT_WR);
            return;
        }

        write(sock, buf, strlen(buf));
    }
}

int itoc(int num, char *str)
{
    char tem[1024];
    int id = 0, id2 = 0;
    while (num)
    {
        int t = num % 10;
        tem[id++] = t + '0';
        num /= 10;
    }
    str[id--] = '\0';
    while (id >= 0)
    {
        str[id2++] = tem[id--];
    }
    return 0;
}
```

编译运行

```shell
$ gcc chat_server.c -D_REENTRANT -o ./bin/chat_server -lpthread
$ ./bin/char_server 9190 
Connect client ID: 127.0.0.1 
Connect client ID: 127.0.0.1 
```



```shell
$ gcc chat_clnt.c -D_REENTRANT -o ./bin/chat_clnt -lpthread
$ ./bin/chat_clnt 127.0.0.1 9190 dog
Connected......
Hi, I am dog
[dog] Hi, I am dog
[cat] Hi, I am cat
```



```shell
$ ./bin/chat_clnt 127.0.0.1 9190 cat
Connected......
[dog] Hi, I am dog
Hi, I am cat               
[cat] Hi, I am cat
```



## 8、习题（参考答案）

![截屏2021-05-21 下午4.27.17](/Users/wangjun/Desktop/截图/截屏2021-05-21 下午4.27.17.png)

![截屏2021-05-21 下午4.27.29](/Users/wangjun/Desktop/截图/截屏2021-05-21 下午4.27.29.png)


（1）为了让单核CPU同时执行多个进程，采用了事件片轮转的方式实现了宏观上的并发执行。即每个进程独占一个CPU核心运行一定时间后，切换为下一个进程执行，由于每个进程执行的时间片极短，因此宏观上感觉所有进程在并发执行。但频繁的进程切换也需要产生较大的开销。

来自百度百科的一段解释：上下文切换

上下文切换 (context switch) , 其实际含义是任务切换, 或者CPU寄存器切换。当多任务内核决定运行另外的任务时, 它保存正在运行任务的当前状态, 也就是CPU寄存器中的全部内容。这些内容被保存在任务自己的堆栈中, 入栈工作完成后就把下一个将要运行的任务的当前状况从该任务的栈中重新装入CPU寄存器, 并开始下一个任务的运行, 这一过程就是context switch。



（2）每个进程都具有独立的内存空间，包括“数据区”，”堆(Heap)“，“栈(Stack)”等。上下文切换时需要更多时间。

而多个线程共享数据区和堆，只独享栈区，在上下文切换时需要的时间更少。

因为多个线程共享数据区，因为可以通过数据区和堆区通信。



（3）进程：一个进程可以通过调用fork函数创建子进程，根据返回值pid区分父子进程，pid==0的部分为子进程执行区域，pid!=0的部分为父进程执行区域，其余部分父子进程共有。子进程结束后，由父进程负责回收其资源，可以通过调用wait,waitpid函数或信号处理方式。

线程：一个进程可以通过pthread_create函数创建多个线程，每个线程共享数据区和堆区，拥有独立的栈区。线程执行完毕由父进程负责回收其资源，通常调用pthread_join或pthread_detach。



（4）c：例如：假设有A，B两个线程，线程A负责向指定内存空间写入数据，线程B负责取走该数据。这是线程A执行的代码块和线程B执行的代码块就构成临界区。

d：只要两个线程同时访问同一个内存区域，就可能构成临界区。



（5）d



（6）Linux中可通过在进程中调用下列函数销毁创建的线程。

* 调用pthread_join函数
* 调用pthread_detach函数



（7）

```c
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

void errorHandling(const char *message);
void *handle_clnt(void *arg);
void *send_msg(char *msg, int len);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutex;
pthread_mutex_t msg_mutex;

int main(int argc, char *argv[])
{
    int str_len, state;
    int serv_sock, clnt_sock;
    char message[BUF_SIZE];
    pthread_t t_id;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    if (argc != 2)
    {
        printf("Usage: %s <port> \n", argv[1]);
        exit(1);
    }

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&msg_mutex, NULL);

    //调用socket函数创建套接字
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        errorHandling("socket() error");
    }
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    //调用bind函数分配IP地址和端口号
    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    {
        errorHandling("bind() error!");
    }

    //调用listen函数将套接字转化为可接收连接状态
    if (listen(serv_sock, 5) == -1)
    {
        errorHandling("listen() error!");
    }
    //write函数用于传输数据，若程序经accept函数运行到本行，说明已经有了连接请求
    //调用accept函数受理连接请求，如果在没有连接请求的情况下调用该函数，则不会返回，直到有连接请求为止。

    while (1)
    {
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
        pthread_mutex_lock(&mutex);
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutex);

        pthread_create(&t_id, NULL, handle_clnt, (void *)&clnt_sock);
        pthread_detach(t_id);
        printf("Connect client ID: %s \n", inet_ntoa(clnt_adr.sin_addr));
    }
    close(serv_sock);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&msg_mutex);
    return 0;
}

void errorHandling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void *handle_clnt(void *arg)
{
    int clnt_sock = *((int *)arg);
    int str_len = 0;
    char msg[BUF_SIZE];
    
    pthread_mutex_lock(&msg_mutex);
    while ((str_len = read(clnt_sock, msg, sizeof(msg))) != 0)
    {
        write(clnt_sock, msg, str_len);
        
    }
    pthread_mutex_unlock(&msg_mutex);

    pthread_mutex_lock(&mutex);
    for (int i = 0; i < clnt_cnt; i++)
    {
        if (clnt_sock == clnt_socks[i])
        {
            while (i++ < clnt_cnt - 1)
            {
                clnt_socks[i] = clnt_socks[i + 1];
            }
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutex);

    close(clnt_sock);
    return NULL;
}


```



（8）如果不同步，会导致发送给A线程的数据，被B线程取走，或者发送给A线程的数据还没被取走，已经被B接收的数据覆盖了。

如果同步，导致同时只能为一位客户端服务，只有当一个客户端断开连接时，消息缓冲才能被下一个线程访问。







```
#include <iostream>

using namespace std;

int main()
{
    int grade;
    int n;
    printf("学生个数: ");
    scanf("%d", &n);

    for (int i = 0; i < n; i++)
    {
        cout << "分数: ";
        cin>>grade;
    
        if(grade<80){
            if(grade>=70){
                cout << "中" <<endl;
            }else if(grade>=60){
                cout<< "及格" <<endl;
            }else{
                cout<< "不及格" <<endl;
            }
        }else{
            if(grade<90){
                cout<< "良" <<endl;
            }else{
                cout<< "优" <<endl;
            }
        }
    }
    return 0;
}
#include <iostream>

using namespace std;

int main()
{
    int grade;
    int n;
    printf("学生个数: ");
    scanf("%d", &n);

    for (int i = 0; i < n; i++)
    {
        cout << "分数: ";
        cin>>grade;
    
        if(grade<80){
            if(grade>=70){
                cout << "中" <<endl;
            }else if(grade>=60){
                cout<< "及格" <<endl;
            }else{
                cout<< "不及格" <<endl;
            }
        }else{
            if(grade<90){
                cout<< "良" <<endl;
            }else{
                cout<< "优" <<endl;
            }
        }
    }
    return 0;
}
#include <iostream>

using namespace std;

int main()
{
    int grade;
    int n;
    printf("学生个数: ");
    scanf("%d", &n);

    for (int i = 0; i < n; i++)
    {
        cout << "分数: ";
        cin>>grade;
    
        if(grade<80){
            if(grade>=70){
                cout << "中" <<endl;
            }else if(grade>=60){
                cout<< "及格" <<endl;
            }else{
                cout<< "不及格" <<endl;
            }
        }else{
            if(grade<90){
                cout<< "良" <<endl;
            }else{
                cout<< "优" <<endl;
            }
        }
    }
    return 0;
}
#include <iostream>

using namespace std;

int main()
{
    int grade;
    int n;
    printf("学生个数: ");
    scanf("%d", &n);

    for (int i = 0; i < n; i++)
    {
        cout << "分数: ";
        cin>>grade;
    
        if(grade<80){
            if(grade>=70){
                cout << "中" <<endl;
            }else if(grade>=60){
                cout<< "及格" <<endl;
            }else{
                cout<< "不及格" <<endl;
            }
        }else{
            if(grade<90){
                cout<< "良" <<endl;
            }else{
                cout<< "优" <<endl;
            }
        }
    }
    return 0;
}
```



# 第 24 章 制作HTTP服务器端

## 1、HTTP概要

### 1.1 连接web服务器

“基于HTTP协议，将网页对应文件传输给客户端的服务器”。

HTTP协议：以超文本传输为目的而设计的应用层协议，这种协议基于TCP/IP实现的协议，我们也可以自己实现HTTP。



### 1.2 HTTP

#### 无状态的Stateless协议

为了在网络环境中同时向大量客户端提供服务，HTTP协议的请求及响应方式设计如图所示：

<img src="https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-23%20%E4%B8%8B%E5%8D%885.25.14.png" alt="截屏2021-05-23 下午5.25.14" style="zoom:50%;" />



从上图可以看出，服务器端响应客户端请求后立即断开了连接。即使同一客户端再次发送请求，服务器端也无法分辨出是原先哪一个，而会以相同的方式处理新请求。因此HTTP又称“无状态的Stateless协议”



为了弥补HTTP无法保持连接的缺点，web编程中通常会使用Cookie和Session技术。

> Cookie：类型为“小型文本文件”，指某些网站为了辨别用户身份而储存在用户本地终端（Client Side）上的数据（通常经过加密）。cookie采用的是键值对结构存储，只不过键和值都是字符串类型。
>
> Session：是服务器程序记录在服务器端的会话消息。



#### 请求消息（Request Message）的结构

Web服务器端需要解析并响应客户端请求。客户端和服务端之间的数据请求方式标准如下图所示：

<img src="https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-23%20%E4%B8%8B%E5%8D%885.38.25.png" alt="截屏2021-05-23 下午5.38.25" style="zoom:50%;" />



请求消息可分为请求行，消息头，消息体等3个部分。

请求行中含有请求方式消息，典型的请求方式有GET和POST。GET主要用于请求数据，POST主要用于传输数据

"GET/index.html HTTP/1.1" 具有如下含义：

请求(GET)index.html文件，希望以1.1版本的HTTP协议进行通信。

请求行只能通过1行(line)。因此服务器端很容易从HTTP请求中提取第一行，并分析请求行中的信息。



#### 响应消息（Response Message）消息

下面介绍Web服务器端向客户端传递的响应消息的结构。响应消息由状态行，头消息，消息体等3个部分构成。该响应消息由状态行，头消息，消息体等3部分构成。状态行中含有关于请求的状态消息。

<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-05-23 下午6.05.42.png" alt="截屏2021-05-23 下午6.05.42" style="zoom: 50%;" />



例如客户端请求index.html文件时，表示index.html文件是否存在，服务器端是否发生问题而无法响应等不同情况的信息将写入状态行。

"HTTP/1.1 200 OK"的含义为：我想用HTTP1.1版本进行响应，你的请求已正确处理(200 OK)

表示“客户端请求的执行结果”的数字称为状态码，典型的有以下几种。

200 OK：成功处理了请求

404 Not Found：请求的文件不存在

400 Bad Request：请求方式错误，请检查

消息头中含有传输的数据类型和长度等信息

最后插入一个空行，通过消息体发送客户端请求的文件数据。



## 2、实现简单的Web服务器端

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define SMALL_BUF 100

void* request_handler(void* arg);
void send_data(FILE* fp, char* ct, char* file_name);
char* content_type(char* file);
void send_error(FILE* fp);
void error_handling(char* message);

int main(int argc, char* argv[]){
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    int clnt_adr_size;
    char buf[BUF_SIZE];
    pthread_t t_id;
    if(argc != 2){
        printf("Usage: %s <PORT>\n",argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));
    if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
        error_handling("bind error");
    
    if(listen(serv_sock, 20)==-1)
        error_handling("listen() error");
    
    while(1){
        clnt_adr_size = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_size);
        printf("Connect Request : %s:%d\n", inet_ntoa(clnt_adr.sin_addr), ntohs(clnt_adr.sin_port));
        pthread_create(&t_id, NULL, request_handler, &clnt_sock);
        pthread_detach(t_id);
    }
    close(serv_sock);
    return 0;
}

void * request_handler(void* arg){
    int clnt_sock = *((int*)arg);
    char req_line[SMALL_BUF];
    FILE* clnt_read;
    FILE* clnt_write;

    char method[10];
    char ct[15];
    char file_name[30];
    clnt_read = fdopen(clnt_sock, "r");        // fdopen 将文件描述符转为FILE指针
    clnt_write = fdopen(dup(clnt_sock), "w");  // dup 为复制文件描述符，
    fgets(req_line, SMALL_BUF, clnt_read);     // fgets从指定流中读取一行
    if(strstr(req_line, "HTTP/")==NULL){
        send_error(clnt_write);
        fclose(clnt_read);
        fclose(clnt_write);
        return NULL;
    }
    strcpy(method, strtok(req_line, " /"));
    strcpy(file_name, strtok(NULL, " /"));
    strcpy(ct, content_type(file_name));
    if(strcmp(method, "GET")!=0){
        send_error(clnt_write);
        fclose(clnt_write);
        fclose(clnt_read);
        return NULL;
    }
    fclose(clnt_read);
    send_data(clnt_write, ct, file_name);
}

void send_data(FILE* fp, char* ct, char* file_name){
    char path[] = "./";
    char protocol[] = "HTTP/1.0 200 OK\r\n";
    char server[] = "Server: Linux Web Server \r\n";
    char cnt_len[] = "Content-length:2048\r\n";
    char cnt_type[SMALL_BUF];
    FILE * send_file;
    char buf[BUF_SIZE];
    sprintf(cnt_type, "Content-type:%s\r\n\r\n",ct);
    char *file_path = malloc(sizeof(char) * (strlen(file_name) + strlen(path) + 1));
    strcpy(file_path, path);
    strcat(file_path, file_name);

    send_file = fopen(file_path,"r");
    if(send_file==NULL){
        send_error(fp);
        return;
    }
    // 传输头信息
    fputs(protocol, fp);
    fputs(server, fp);
    fputs(cnt_len,fp);
    fputs(cnt_type, fp);

    // 传输请求数据
    while(fgets(buf, BUF_SIZE, send_file)!=NULL){
        fputs(buf, fp);
        fflush(fp);
    }
    fflush(fp);
    fclose(fp);
}

char * content_type(char* file){
    char extension[SMALL_BUF];
    char file_name[SMALL_BUF];
    strcpy(file_name, file);
    strtok(file_name, ".");
    strcpy(extension, strtok(NULL, "."));

    if(!strcmp(extension, "html") || strcmp(extension, "htm"))
        return "text/html";
    else
        return "text/plain";
}

void send_error(FILE *fp){
    char protocol[] = "HTTP/1.0 404 Not Found\r\n";
    char date[] = " Tue, 10 Jul 2012 06:50:15 GMT\r\n";
    char cnt_len[] = "Content-Length: 2048\r\n";
    char cnt_type[] = "Content-Type: text/html;charset=utf-8\r\n\r\n";

    char server[] = "Server: Linux Web Werver \r\n";
    char content[] = "<html><head><meta charset=\"utf-8\"><title>NETWORK</title></head>"
                                                          "<body><front size=+5><br>发生错误！查看请求文件名和请求方式！</front></body>"
                                                          "</html>";
    fputs(protocol, fp);
    fputs(date, fp);
    fputs(server, fp);
    fputs(cnt_len,fp);
    fputs(cnt_type, fp);
    fputs(content, fp);
    fflush(fp);
}

void error_handling(char* message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);

}
```



编译运行

```shell
$ gcc -g  webserver_linux.c -D_REENTRANT -o ./bin/webserver_linux -lpthread
$ ./bin/webserver_linux  9190
Connect Request : 10.211.55.2:55445
Connect Request : 10.211.55.2:55450
```

接下来就可以通过浏览器访问了

http://10.211.55.22:9190/index.html



## 3、习题（参考答案）

![截屏2021-05-23 下午6.27.23](https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-23%20%E4%B8%8B%E5%8D%886.27.23.png)

![截屏2021-05-23 下午6.23.06](https://raw.githubusercontent.com/wangjunstf/pics/main/uPic/%E6%88%AA%E5%B1%8F2021-05-23%20%E4%B8%8B%E5%8D%886.23.06.png)



（1）a，b：服务器端响应客户端请求后立即断开连接，e

（2）a，目前广泛使用HTTP协议基于TCP协议，只能使用TCP协议实现。下一代HTTP3，基于UDP，可以使用UDP协议实现。

（3）因为HTTP协议是"无状态的states协议"，每次请求都需要经过套接字的创建和销毁过程。

epoll对多核/多线程的支持不够好,