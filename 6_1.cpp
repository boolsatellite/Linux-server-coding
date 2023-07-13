//
// Created by satellite on 2/07/2023.
// CGI服务器原理


#include "sys/socket.h"
#include "sys/types.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "assert.h"
#include "stdio.h"
#include "unistd.h"
#include "netdb.h"
#include "stdlib.h"
#include "errno.h"
#include "string.h"

int main(int argc , char * argv[])
{
    if(argc <= 2)
    {
        printf("usage : %s ip prot \n", basename(argv[0]));
        return 1;
    }
    const char * ip = argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET,ip,&address.sin_addr.s_addr);
    int sock = socket(AF_INET,SOCK_STREAM,0);
    assert(sock >= 0);
    int ret = bind(sock,(struct sockaddr*)&address,sizeof(address));
    assert(ret != -1);
    ret = listen(sock,5);
    assert(ret != -1);
    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    int connfd = accept(sock,(struct sockaddr*)&client,&client_addrlength);
    if(connfd < 0)
        printf("errno is : %d \n",errno);
    else
    {
        close(STDOUT_FILENO);
        dup(connfd);
        printf("abcd \n");
        close(connfd);
    }
    close(sock);
    return 0;
}

/*
我们先关闭标准输出文件描述符
STDOUT_FILENO（其值是1），然后复制socket文件描述符connfd。
因为dup总是返回系统中最小的可用文件描述符，所以它的返回值实际
上是1，即之前关闭的标准输出文件描述符的值。这样一来，服务器输
出到标准输出的内容（这里是“abcd”）就会直接发送到与客户连接对
应的socket上，因此printf调用的输出将被客户端获得
 * */