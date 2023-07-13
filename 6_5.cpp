//
// Created by satellite on 7/07/2023.
// 同时输出数据到终端和文件的程序

#include "assert.h"
#include "stdio.h"
#include "unistd.h"
#include "errno.h"
#include "string.h"
#include "fcntl.h"

int main(int argc , char * argv[])
{
    if(argc != 2)
    {
        printf("usage : <file> \n", basename(argv[0]));
        return 1;
    }
    int filefd = open(argv[1],O_CREAT|O_WRONLY|O_TRUNC,0666);
    assert(filefd > 0);
    int pipefd_stdout[2];
    int ret = pipe(pipefd_stdout);
    assert(ret != -1);
    ret = splice(STDIN_FILENO,NULL,pipefd_stdout[1],NULL,32768,SPLICE_F_MOVE|SPLICE_F_MORE);
    assert(ret != -1);
    ret = tee(pipefd_stdout[0],pipefd_stdout[1],32768,SPLICE_F_MORE|SPLICE_F_MOVE);
    assert(ret != -1);
    ret = splice(pipefd_stdout[0],NULL,filefd,NULL,32768,SPLICE_F_MOVE|SPLICE_F_MORE);
    assert(ret != -1);
    ret = splice(pipefd_stdout[0],NULL,STDOUT_FILENO,NULL,32768,SPLICE_F_MORE|SPLICE_F_MOVE);
    assert(ret != -1);

}

