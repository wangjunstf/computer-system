# 第6章 基于UDP的服务器端/客户端

## 一、UDP概述

### 1.1 TCP和UDP的区别

1. UDP位于4层TCP/IP协议栈中，从上往下数第二层，与TCP协议处于相同的位置。

2. ==TCP是可靠的网络通信协议==，每次通信都要经过3次握手建立连接，中间的数据传输也会通过ACK确认，SEQ给数据包编号，有确认、窗口、重传、拥塞控制机制，来确保数据的安全传输。最后断开连接也需要经过四次挥手来安全断开连接。可以看到，即使发送一条数据量很小的信息，也需要经过多次额外的数据包来确认信息的传输，==传输效率低==。

3. ==UDP是不可靠的网络通信协议==，相比TCP，UDP则不需要额外的数据包来确认，而是直接将数据包发送给对方。因而==数据传输效率要高于TCP==。

4. ==TCP区分服务器端和客户端，UDP不区分服务器端和客户端==。
5. TCP首部开销20字节，UDP首部开销8字节。
6. 大多数情况，TCP的传输速度无法超过UDP，但也存在特殊情况：每次传输的数据量越大，TCP的传输速率就越接近UDP的传输速率。

### 1.2 TCP的优缺点

**TCP的优点**：安全可靠稳定

**TCP的缺点**：由于需要对数据包进行额外的确认，因而传输效率较低，占用系统资源高，易被攻击。

> 一台计算机能与其他计算机建立的TCP连接是有限的，当有恶意攻击者占用了全部TCP连接，合法用户就无法再与服务器建立连接。这就是拒绝服务攻击。例如DDOS，CC等攻击。

**应用场景**：

* 数据量较小且需要百分百数据不丢失的数据传输，例如压缩包的传输，用户的交易信息等。
* 对数据要求准确无误传输，例如HTTP，HTTPS，FTP，远程连接等
* 基于邮件的POP，SMTP等

### 1.3 UDP的优缺点

**UDP优点**：传输效率高，例如网络实时传输视频或音频。

**UDP缺点**：不可靠的数据传输，易发生数据丢失或错误。

**应用场景**：

* 适用于数据量较大，对传输效率要求较高，允许少量数据丢失或损坏的数据传输
* 例如网络直播，语音通话等



## 二、UDP的工作原理

### 2.1 UDP的工作原理

UDP是通过数据包的形式发送到目标主机，对UDP而言，每次只发送一个数据包，每个数据包都是独立的一条数据，数据包与数据包之间没有直接关联。

每台主机可通过一个套接字给多个主机发送数据，也可以由一个套接字接收多个主机发送的数据。

### 2.2 执行过程

1. 创建套接字

   ```c
   int serv_sock =  socket(PF_INET, SOCK_DGRAM, 0); 
   //成功时返回文件描述符，失败时返回-1
   /*
   	PF_INET 指IPv4
   	SOCK_DGRAM UDP协议
   */
   ```

2. 给套接字绑定地址信息

   ```c
   bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))
   //成功时返回0，失败时返回-1
   /*
   	serv_sock 发送端或接收到文件描述符，serv_adr本机地址信息结构体变量名
   */
   ```

3. 发送数据

   ```c
   #include <sys/socket.h>
   
   ssize_t sendto(int sock, void *buf, size_t nbytes, int flags, struct sockaddr *to, socklen_t addrlen);
   //成功时返回传输的字节数，失败时返回-1
   
   /*
   	sock 传输数据的UDP套接字文件描述符
   	buf 保存待传数据的缓冲区地址
   	nbytes 传输的字节数
   	flags 可选项参数，若无则传0
   	to 目标地址信息的结构体变量的地址值
   	addrlen to的变量大小
   */
   ```

4. 接收数据

   ```c
   #include <sys/socket.h>
   
   ssize_t recvfrom(int sock, void *buf, size_t nbytes, int flags, struct sockaddr *from, socklen_t addrlen);
   //成功时返回接收的字节数，失败时返回-1
   
   /*
   	sock 传输数据的UDP套接字文件描述符
   	buf 保存接收数据的缓冲区地址
   	nbytes 传输的字节数
   	flags 可选项参数，若无则传0
   	from 发送端信息的结构体变量的地址值
   	addrlen to的变量大小
   */
   
   ```



### 2.3 基于UDP的回声服务器端/客户端

描述：编写服务器端程序和客户端程序，客户端向服务器端发送一条信息，服务器端将信息原路返回，就像回声一样。

udp_echo_server.c

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 20
void error_handling(char* message);

int main(int argc, char* argv[]){
    int serv_sock;
    char message[BUF_SIZE];
    int str_len;
    socklen_t clnt_sock_len;

    struct sockaddr_in serv_adr, cln_adr;

    if(argc!=2){
        printf("Usage:%s <port>\n",argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_DGRAM,0);
    if(serv_sock == -1){
        error_handling("UDP creation is error");
    }

    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1){
        error_handling("bind() error");
    }

    while(1){
        clnt_sock_len = sizeof(cln_adr);
        str_len = recvfrom(serv_sock, message, BUF_SIZE,0,(struct sockaddr*)&serv_adr, &clnt_sock_len);
        sendto(serv_sock, message,str_len,0,(struct sockaddr*)&serv_adr, clnt_sock_len);
    }

    close(serv_sock);

    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n',stderr);
    exit(1);
}

```



udp_echo_client.c

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30

void error_handling(char *message);

int main(int argc, char* argv[]){
    int sock;
    char message[BUF_SIZE];
    int str_len;
    socklen_t adr_len;

    struct sockaddr_in serv_adr, from_adr;

    if(argc!=3){
        printf("Usage: %s <IP> <PORT>\n",argv[1]);
        exit(1);
    }

    sock = socket(AF_INET, SOCK_DGRAM,0);
    if(sock==-1){
        error_handling("socket() error");
    }

    memset(&serv_adr, 0 , sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[2]));

    while(1){
        fputs("Insert message(q to quit)",stdout);
        fgets(message,sizeof(message),stdin);

        if(!strcmp("q\n",message)||!strcmp("Q!\n",message)){
            break;
        }

        sendto(sock, message, strlen(message),0, (struct sockaddr*)&serv_adr, sizeof(serv_adr));

        adr_len = sizeof(from_adr);
        str_len = recvfrom(sock, message, BUF_SIZE, 0, (struct sockaddr*)&from_adr, &adr_len);

        message[str_len] = 0;
        printf("Message from server:%s",message);
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



**编译并执行**

```shell
$ gcc udp_echo_server.c /bin/udp_echo_server
$ ./bin/udp_echo_server 9190

$ gcc udp_echo_client.c /bin/udp_echo_client
$ ./bin/udp_echo_client 127.0.0.1 9190
Insert message(q to quit)hello
Message from server:hello
Insert message(q to quit)hello wold
Message from server:hello wold
Insert message(q to quit)
```

