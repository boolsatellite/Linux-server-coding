//
// Created by satellite on 13/07/2023.
// 使用EPOLLONESHOT事件

#include "sys/types.h"
#include "sys/socket.h"
#include "arpa/inet.h"
#include "assert.h"
#include "stdio.h"
#include "unistd.h"
#include "errno.h"
#include "string.h"
#include "fcntl.h"
#include "stdlib.h"
#include "sys/epoll.h"
#include "pthread.h"

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 1024

struct fds
{
    int epollfd;
    int sockfd;
};

int setnoblocking(int fd)
{
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | SPLICE_F_NONBLOCK;
    fcntl(fd, F_SETFL , new_option);
    return old_option;
}

//将fd上的EPOLLIN和EPOLLET事件注册到epollfd指示的epoll内核事件表中，参
//数oneshot指定是否注册fd上的EPOLLONESHOT事件
void addfd(int epollfd , int fd , bool oneshot)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    if(oneshot)
    {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    setnoblocking(fd);
}

//*重置fd上的事件。这样操作之后，尽管fd上的EPOLLONESHOT事件被注册，但是操
//作系统仍然会触发fd上的EPOLLIN事件，且只触发一次
void reset_oneshot(int epollfd , int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
}

//工作线程
void * worker(void * arg)
{
    int sockfd = ((fds*)arg) -> sockfd;
    int epollfd = ((fds*)arg) -> epollfd;
    printf("start new thread to receive data on fd : %d \n",sockfd);
    char buf[BUFFER_SIZE];
    memset(buf,'\0',BUFFER_SIZE);
    while(1)
    {
        int ret = recv(sockfd,buf,BUFFER_SIZE-1,0);
        if(ret == 0)
        {
            close(sockfd);
            printf("foreiner closed the connection \n");
            break;
        }
        else if(ret < 0)
        {
            if(errno == EAGAIN)
            {
                reset_oneshot(epollfd,sockfd);
                printf("read later \n");
                break;
            }
        }
        else
        {
            printf("get content : %s \n",buf);
            sleep(5);
        }
    }
    printf("end thread receiving data on fd : %d \n",sockfd);
}

int main(int argc , char * argv[])
{
    if(argc  <= 2 )
    {
        printf("usage : %s ip port", basename(argv[0]));
        return 1;
    }
    const char * ip = argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET,ip,&address.sin_addr);
    int listenfd = socket(AF_INET,SOCK_STREAM,0);
    bind(listenfd,(struct sockaddr*)&address,sizeof(address));
    listen(listenfd,5);
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    addfd(epollfd, listenfd , false);
    while(1)
    {
        int ret = epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);
        if(ret < 0)
        {
            printf("epoll failure \n");
            break;
        }
        for(int i=0;i<ret;i++)
        {
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd,(struct sockaddr*)&client_address,
                        &client_addrlength);
                addfd(epollfd,connfd, true);
            }
            else if(events[i].events & EPOLLIN)
            {
                pthread_t thread;
                fds fds_for_new_worker;
                fds_for_new_worker.epollfd = epollfd;
                fds_for_new_worker.sockfd = sockfd;
                pthread_create(&thread,NULL,worker,(void*)&fds_for_new_worker);
            }
            else
            {
                printf("something else happened \n");
            }
        }
    }
    close(listenfd);
    return 0;
}











