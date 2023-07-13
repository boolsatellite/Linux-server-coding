//
// Created by satellite on 13/07/2023.
// 非阻塞connect

#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "stdlib.h"
#include "assert.h"
#include "stdio.h"
#include "time.h"
#include "errno.h"
#include "fcntl.h"
#include "sys/ioctl.h"
#include "unistd.h"
#include "string.h"
#define BUFFER_SIZE 1023

int setnoblocking(int fd)
{
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

//超时连接函数，参数分别是服务器IP地址、端口号和超时时间（毫秒）。函数成功时
//返回已经处于连接状态的socket，失败则返回-1

int unblock_connect(const char * ip , int port , int time)
{
    int ret = 0;
}

