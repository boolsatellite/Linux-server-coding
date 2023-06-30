# Linux-server-coding



## linux网络编程基础API

### 字节序

发送端总是把要发送的数据转化成大端字节序数据后再发送，而接收端知道对方传送过来的数据总是采用大端字节序，所 以接收端可以根据自身采用的字节序决定是否对接收到的数据进行转换（小端机转换，大端机不转换）。因此大端字节序也称为网络字节序，它给所有接收数据的主机提供了一个正确解释收到的格式化数据的保证。即使是同一台机器上的两个进程（比如一个由C语言编写，另一个由JAVA编写）通信，也要考虑字节序的问题（JAVA虚拟机采用大端字节序）。

linux体统了4个函数用于完成主机字节序和网络字节序之间的转换

```c
#include <netinet/in.h>
unsigned long int htonl(unsigned long int hostlong);
unsigned short int htons(unsigned short int hostshort);
unsigned long int ntohl(unsigned long int netlong);
unsigned short int ntohs(unsigned short int netshort);
```

### 通用socket地址

socket网络编程中表示socket地址的是结构体```sockaddr``，定义：

```c
#include <bits/socket.h>
struct sockaddr{
	sa_family_t sa_family;   //存放地址族
	char sa_data[14];        //存放socket地址值，不同协议族的地址值具有不同的涵义和长度
}
```

| 协议族   | 地址值含义和长度                                             |
| -------- | ------------------------------------------------------------ |
| PF_UNIX  | 文件路径名，长度可达到108字节                                |
| PF_INET  | 16 bit 端口号 32 bit IPV4地址，共6字节                       |
| PF_INET6 | 16 bit 端口号 32 bit 流标志，128 bit IPV6地址，32bit范围ID，共26字节 |

14字节的sa_data根本无法完全容纳多数协议族的地 址值。因此，Linux定义了下面这个新的通用socket地址结构体：

```c
#include <bits/socket.h>
struct sockaddr_storage
{
	sa_family_t sa_family;
	unsigned long int __ss_align;
	char __ss_padding[128 - sizeof(__ss_align)];
}
```

### 专用socket地址

上述地址结构若要获取 IP 和 端口，需要进行繁琐的按位操作，Linux为各个协议族提供了专门的socket地址结构

```c
#include <sys/un.h>
struct sockaddr_un
{
	sa_family_t sin_family;    //地址族:AF_UNIX
	char sun_path[108];        //文件路径名
}
struct sockaddr_in
{
	sa_family_t sin_family;        //地址族:AF_INET
	u_int16_t sin_port;			  //端口号
	struct in_addr sin_addr;       //IPV4地址结构体
}
struct in_addr
{
	u_int32_t s_addr;				/*IPv4地址，要用网络字节序表示*/
};
struct sockaddr_in6
{
	sa_family_t sin6_family;	/*地址族：AF_INET6*/
	u_int16_t sin6_port;		/*端口号，要用网络字节序表示*/
	u_int32_t sin6_flowinfo;	/*流信息，应设置为0*/
	struct in6_addr sin6_addr;	/*IPv6地址结构体，见下面*/
	u_int32_t sin6_scope_id;	/*scope ID，尚处于实验阶段*/
};
struct in6_addr
{
	unsigned char sa_addr[16];	/*IPv6地址，要用网络字节序表示*/
}；
```

所有专用socket地址（以及sockaddr_storage）类型的变量在实际使 用时都需要转化为通用socket地址类型sockaddr（强制转换即可），因为所有socket编程接口使用的地址参数的类型都是sockaddr。

### IP地址转换函数

点分十进制字符串和网络字节序证书表示IPV4地址之间的转换函数

```c
#include <arpa/inet.h>
in_addr_t inet_addr(const char * strptr);
/*将用点分十进制字符串表示的IPv4地址转化为用网络
字节序整数表示的IPv4地址。它失败时返回INADDR_NONE。*/

int inet_aton(const char * cp , struct in_addr * inp);
/*将点分十进制转化为网络字节序，并将结果储存在inp所指位置，成功返回 1 ， 失败返回 0*/

char * inet_ntoa(struct in_addr in);
/*将用网络字节序整数表示的IPv4地址转化为用点分十进制字符串表示的IPv4地址。
但需要注意的是，该函数内部用一个静态变量存储转化结果，
函数的返回值指向该静态内存，因此inet_ntoa是不可重入的。*/
```

新更新的函数，并且还能同时适用于 IPV4 和 IPV6

```c
#include <arpa/inet.h>
int inet_pton(int ad , const char * src , void * dest);
/*
用于将点分十进制/16进制 ip 转化为网络字节序，并把转换结果存储于dst指向的内存中。
af用于指定地址族，成功时返回1，失败则返回0并设置errno
*/
const char * intet_ntop(int af , const void * src , char * dst , socklen_t cnt);
/*
用于将网络字节序转化为对应的 ip 地址，最后一个参数cnt指定目标存储单元的大小
可以使用下面两个宏定义：
#include <netinet/in.h>
#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46

inet_ntop成功时返回目标存储单元的地址，失败则返回NULL并设置errno。
*/
```

### 创建socket

```c
#include <sys/types.h>
#include＜sys/socket.h＞
int socket(int domain,int type,int protocol);
/*
domain:系统使用那个底层协议族，TCP/IP - PF_INET		UNIX本地 - PF_UNIX
type:指定服务类型， 流服务 - SOCK_STREAM		数据报 - SOCK_UGRAM
自Linux内核版本2.6.17起，type参数可以接受上
述服务类型与下面两个重要的标志相与的值：SOCK_NONBLOCK和
SOCK_CLOEXEC。它们分别表示将新创建的socket设为非阻塞的，以
及用fork调用创建子进程时在子进程中关闭该socket。
protocol: 前两个参数已经完全决定了它的值）。几乎在所有情况下，我们都应该把它设置为0

socket系统调用成功时返回一个socket文件描述符，失败则返回-1并设置errno
*/
```

### 命名socket

将一个socket与socket地址绑定称为给 socket命名。

```c
#include <sys/types.h>
#include <sys/socket.h>
int bind(int sockfd , const struct sockaddr * my_addr , socklen_t addrlen_);
/*
将myaddr所指的socket地址分配给未命名的sockfd，addr是socket地址的长度
bind成功时返回0，失败则返回-1并设置errno。
常见errno有两个值：EACCES EADDRINUSE
EACCES:绑定地址是受保护的，仅root用户可以访问，如:普通用户将socket绑定到知名端口时会返回EACCES错误
EADDRINUSE:被绑定的地址正在使用，如:嫁给你socket绑定到一个处于TIME_WAIT状态的socket地址
*/
```

### 监听socket

socket被命名后还不能马上接受客户连接，还需要使用listen系统调用创建监听队列存放待处理的客户连接

```c
#include <sys/socket.h>
int listen(int sockfd , int backlog);
/*
sockfd:被监听的socket，backlog参数提示内核监听队列的最大长度
监听队列的长度如果超过backlog，服务器将不受理新的客户连接，客户端也将收到ECONNREFUSED错误信息。
成功时返回0，失败则返回-1并设置errno。
*/
```

### 接受连接

该系统调用从listen监听队列中接受一个连接

```c
#include <sys/types.h>
#include <sys/socket.h>
int accept(int sockfd , struct sockaddr * addr , socklen_t * addrlen);
/*
sockfd是执行过listen被监听的socket，addr参数用户获取连接远端的socket地址结构，
该socket地址的长度有参数addrlen参数指出。accept成功时返回一个新的连接socket,用于标识该连接
失败返回 - 并设置errno
*/
```

```accept```只是从监听队列中取出连接，不关心连接处于何种状态，更不关心网络状态的变化

### 发起连接

客户端通过connect函数主动连接服务器

```c
#include <sys/types.h>
#include <sys/socket.h>
int connect(int sockfd , const struct sockaddr * serv_addr , socklen_t addrlen);
/*
connect成功返回 0 一旦成功sockfd就唯一的标识了这个连接，失败返回 -1并设置errno
常见的errno有 ECONNREFUSED:目标端口不存在	ETIMEDOUT:连接超时
*/
```

### 关闭连接

关闭连接就是关闭对应的socket

```c
#include <unistd.h>
int close(int fd);
/*fd参数是待关闭的socket。不过，close系统调用并非总是立即关闭
一个连接，而是将fd的引用计数减1。只有当fd的引用计数为0时，才真
正关闭连接。多进程程序中，一次fork系统调用默认将使父进程中打开
的socket的引用计数加1，因此我们必须在父进程和子进程中都对该
socket执行close调用才能将连接关闭。*/
```

无论如何都要立即终止连接而不是将引用计数减一，shutdown函数

```c
#include <sys/socket.h>
int shutdown(int sockfd , int howto);
/*sockfd是待关闭的socket，howto决定了shutdown的行为：
SHUT_RD:关闭sockfd读端，应用程序不能再针对socket文件描述符执行读操作，并且socket接收缓冲区中的数据都被丢掉
SHUT_WR:关闭sockfd写这一半，sockfd的发送缓冲区中的数据会在真正关闭连接之前全部发送出去，应用程序不可再对该socket文件描述符执行写操作。连接属于半关闭状态
SHUT_RDWR:同时关闭sockfd上的读和写
shutdown成功返回 0 失败返回 -1 并设置errno */
```

### 数据读写

文件读写 read write 同样适用于socket，但socket提供了专门函数用于数据读写，其中适用于TCP流数据的系统调用：

```c
#include <sys/types.h>
#include <sys/socket.h>
ssize_t recv(int sockfd , void * buf , size_t len , int flags);
ssize_t send(int sockfd , const void * buf , size_t len , int flags);

/*
recv读取sockfd上的数据，buf和len分别指定了缓冲区的位置和大小，成功返回实际读取到的数据长度，当返回0时意味着通信对方关闭了连接，失败返回 -1 并设置errno

send往sockfd中填写数据，buf和len分别指定了缓冲区的位置和大小，成功返回实际写入数据的大小，失败返回 -1 并设置errno

flag为数据收发提供额外的控制
*/
```

























