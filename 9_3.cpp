//
// Created by satellite on 13/07/2023.
//

#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "assert.h"
#include "stdio.h"
#include "unistd.h"
#include "errno.h"
#include "string.h"
#include "errno.h"
#include "fcntl.h"
#include "stdlib.h"
#include "sys/epoll.h"
#include "pthread.h"

#define MAX_EVENT_UNMBER 1024
#define BUFFER_SIZE 10

int setnoblocking(int fd)    //将文件描述符设置位非阻塞
{
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | O_NONBLOCK;   //设置非阻塞
    fcntl(fd,F_SETFD,new_option);
    return old_option;
}
//将文件描述符fd上的EPOLLIN注册到epollfd指向的事件表中，参数enable_et指定是否对fd启用ET模式
void addfd(int epollfd , int fd , bool enable_et)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if(enable_et)
    {
        event.events |= EPOLLET;
    }
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    setnoblocking(fd);
}

// LT模式的工作流程
void lt(epoll_event * events , int number , int epollfd , int listenfd)
{
    char buf[BUFFER_SIZE];
    for(int i=0;i<number;i++)
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd)
        {
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);
            int connfd = accept(listenfd,(struct sockaddr*)&client_address,&client_addrlength);
            addfd(epollfd,connfd, false);  //禁用ET模式
        }
        else if(events[i].events & EPOLLIN)  //当socket读缓存中还有未读出数据
        {
            printf("event trigger once \n");
            memset(buf,'\0',BUFFER_SIZE);
            int ret = recv(sockfd , buf , BUFFER_SIZE-1 , 0);
            if(ret <= 0)
            {
                close(sockfd);
                continue;
            }
            printf("get %d bytes of content : %s \n",ret , buf);
        }
        else
        {
            printf("something else happened \n");
        }
    }
}

void et(epoll_event * events , int number , int epollfd , int listenfd)
{
    char buf[BUFFER_SIZE];
    for(int i=0 ; i<number ; i++)
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd)
        {
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);
            int connfd = accept(listenfd,(struct sockaddr*)&client_address,&client_addrlength);
            addfd(epollfd,connfd, true);
        }
        else if(events[i].events & EPOLLIN)
        {
            //由于是ET模式，所以我们要循环读取数据，确保把socket中所有数据都取出
            printf("event trigger once \n");
            while(1)
            {
                memset(buf,'\0',sizeof(buf));
                int ret = recv(events[i].data.fd,buf,BUFFER_SIZE-1,0);
                if(ret < 0)
                {
                    if(errno == EAGAIN)
                    {
                        printf("read later");
                        break;  //只是把接收缓冲区的数据读完，并不是将全部数据读完
                    }
                    close(sockfd);
                    break;
                }
                else if(ret == 0 )
                {
                    close(sockfd);
                }
                else
                {
                    printf("get %d bytes of content : %s \n",ret , buf);
                }
            }
        }
        else
        {
            printf("somting eles happend \n");
        }
    }
}

int main(int argc , char * argv[])
{
    if(argc <= 2)
    {
        printf("usage ip port\n");
        return 1;
    }
    const char * ip = argv[1];
    int port = atoi(argv[2]);
    int ret = 0;
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET,ip,&address.sin_addr);
    int listenfd = socket(AF_INET,SOCK_STREAM,0);
    assert(listenfd >= 0);
    bind(listenfd,(struct sockaddr*)&address,sizeof(address));
    listen(listenfd,5);
    epoll_event events[MAX_EVENT_UNMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    addfd(epollfd,listenfd, true);
    while(1)
    {
        int ret = epoll_wait(epollfd , events , MAX_EVENT_UNMBER , -1);
        if(ret < 0)
        {
            printf("epoll failure \n");
            break;
        }
        lt(events,ret,epollfd,listenfd);
    }
    close(listenfd);
    return 0;
}




















