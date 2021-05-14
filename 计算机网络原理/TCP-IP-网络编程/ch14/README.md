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



1.3 实现多播 Sender 和 Receiver

该示例的运行场景如下：

* Sender：向AAA组（Broadcastin）文件中保存的新闻信息
* Receiver：接收传递到AAA组的新闻信息



news_sender.c

 