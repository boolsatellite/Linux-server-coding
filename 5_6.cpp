//
// Created by satellite on 30/06/2023.
// 发送带外数据

#include "sys/socket.h"
#include "sys/types.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "assert.h"
#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "errno.h"
#include "string.h"

int main(int argc , char * argv[])
{
    if(argc <= 2)
    {
        printf("usage : %s ip port \n",basename(argv[0]));
        return 1;
    }
    const char * ip = argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in server_address;
    bzero(&server_address,sizeof(server_address));
    server_address.sin_family = PF_INET;
    server_address.sin_port = htons(port);
    inet_pton(PF_INET,ip,&server_address.sin_addr.s_addr);
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    assert(sockfd >= 0);
    if(connect(sockfd,(struct sockaddr*)&server_address,sizeof(server_address)) < 0)
    {
        printf("connection failed \n");
    }
    else
    {
        const char * oob_data = "abc";
        const char * normal_data = "123";
        send(sockfd,normal_data, strlen(normal_data),0);
        send(sockfd,oob_data,strlen(oob_data),MSG_OOB);
        send(sockfd,normal_data, strlen(normal_data),0);
    }
    close(sockfd);
    return 0;
}










