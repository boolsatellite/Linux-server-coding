//
// Created by satellite on 30/06/2023.
//

#include "arpa/inet.h"
#include "stdio.h"

int main()
{
    in_addr_t i = inet_addr("127.0.0.1");
    char * p1 = inet_ntoa(i);
}