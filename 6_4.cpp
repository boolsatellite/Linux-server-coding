//
// Created by satellite on 7/07/2023.
// 使用splice函数实现回射服务器

#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "assert.h"
#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "errno.h"
#include "string.h"
#include "fcntl.h"

int main(int argc ,  char * argv[])
{
    if(argc <= 2)
    {
        printf("usage ip prort \n", basename(argv[0]));
        return 1;
    }
    const char * ip = argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port = htons(port);
    int sock = socket(AF_INET,SOCK_STREAM,0);
    bind(sock,(struct sockaddr*)&address,sizeof(address));
    listen(sock,5);
    struct sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    int connfd = accept(sock,(struct sockaddr*)&client,&client_addrlength);
    if(connfd < 0)
    {
        printf("errno is %d \n",errno);
    }
    else
    {
        int pipfd[2];
        int ret = pipe(pipfd);
        // 将connfd上流入的客户数据定向到管道中
        ret = splice(connfd,NULL,pipfd[1],NULL,32768,SPLICE_F_MORE|SPLICE_F_MOVE);
        assert(ret != -1);
        ret = splice(pipfd[0],NULL,connfd,NULL,32768,SPLICE_F_MOVE|SPLICE_F_MORE);
        assert(ret != -1);
        close(connfd);
    }
    close(sock);
    return 0;
}
/*
我们通过splice函数将客户端的内容读入到pipefd[1]中，然后再使
用splice函数从pipefd[0]中读出该内容到客户端，从而实现了简单高效
的回射服务。整个过程未执行recv/send操作，因此也未涉及用户空间和
内核空间之间的数据拷贝
 * */
