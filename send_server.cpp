//
// Created by 覃立中 on 2021/2/27.
//g++ -g -o send_server send_server.cpp
//

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <errno.h>
/*
 * socket 阻塞模式下的 send 行为
 */
void test_0()
{
    printf("-----------------------test_0-----------------------\n");
    //1.创建一个侦听socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        perror("socket error");
        exit(-1);
    }

    //2.初始化服务器地址
    struct sockaddr_in bindaddr;
    bzero(&bindaddr, sizeof(bindaddr));

    bindaddr.sin_family = AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bindaddr.sin_port = htons(3000);
    if (bind(listenfd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) == -1)
    {
        close(listenfd);
        perror("bind error");
        exit(-1);
    }

    //3.启动侦听
    if (listen(listenfd, SOMAXCONN) == -1)
    {
        close(listenfd);
        perror("listen error");
        exit(-1);
    }

    while (true)
    {
        struct sockaddr_in clientaddr;
        bzero(&clientaddr, sizeof(clientaddr));
        socklen_t clientaddrlen = sizeof(clientaddr);
        //4. 接受客户端连接
        int clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddrlen);
        if (clientfd != -1) //连接成功
        {
            //只接受连接，不调用recv收取任何数据
            std::cout << "accept success, client ip:port--->"<< inet_ntoa(clientaddr.sin_addr) << ":" << ntohs(clientaddr.sin_port) << std::endl;
        }
        else
        {
            perror("accept error");
            continue;
        }
    }

    //7.关闭侦听socket
    close(listenfd);
}


int main(int argc, char* argv[])
{
    test_0();
    //test_1();
    return 0;
}