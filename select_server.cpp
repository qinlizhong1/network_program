//
// Created by 覃立中 on 2021/2/27.
//g++ -g -o select_server select_server.cpp
//

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/time.h>
#include <vector>
#include <errno.h>

//自定义代表无效fd的值
#define INVALID_FD -1
/*
 */
void test_0()
{
    printf("-----------------------test_0-----------------------\n");
    //创建一个侦听socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        perror("socket error");
        exit(-1);
    }

    struct sockaddr_in bindaddr;
    bzero(&bindaddr, sizeof(bindaddr));
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_port = htons(3000);
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) == -1)
    {
        perror("bind error");
        close(listenfd);
        exit(-1);
    }

    //启动侦听
    if (listen(listenfd, SOMAXCONN) == -1)
    {
        perror("linsten error");
        close(listenfd);
        exit(-1);
    }

    //存储客户端socket的数组
    std::vector<int> clientfds;
    int maxfd = listenfd;

    while (true)
    {
        fd_set readset;
        FD_ZERO(&readset);

        //将侦听socket加入到待检测的可读事件中去
        FD_SET(listenfd, &readset);

        //将客户端fd加入到待检测的可读事件中去
        int clientfdslength = clientfds.size();
        for (int i = 0; i < clientfdslength; ++i)
        {
            if (clientfds[i] != INVALID_FD)
            {
                FD_SET(clientfds[i], &readset);
            }
        }

        timeval tm;
        tm.tv_sec = 1;
        tm.tv_usec = 0;
        //暂且只检测可读事件，不检测可写和异常事件
        int ret = select(maxfd + 1, &readset, NULL, NULL, &tm);
        if (ret == -1)
        {
            //出错，退出程序。
            if (errno != EINTR)
            {
                perror("select error");
                break;
            }
        }
        else if (ret == 0)
        {
            //select 函数超时，下次继续
            continue;
        }
        else //检测有事件发生
        {
            if (FD_ISSET(listenfd, &readset)) //监听套接字可读
            {
                //侦听socket的可读事件，则表明有新的连接到来
                struct sockaddr_in clientaddr;
                socklen_t clientaddrlen = sizeof(clientaddr);
                //4. 接受客户端连接
                int clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddrlen);
                if (clientfd == -1)
                {
                    //接受连接出错，退出程序
                    break;
                }

                //只接受连接，无需调用recv收取任何数据
                std:: cout << "accept a client connection, fd: " << clientfd << std::endl;
                clientfds.push_back(clientfd);
                //记录一下最新的最大fd值，以便作为下一轮循环中select的第一个参数
                if (clientfd > maxfd)
                    maxfd = clientfd;
            }
            else
            {
                char recvbuf[64]; //假设对端发来的数据长度不超过63个字符
                int clientfdslength = clientfds.size();
                for (int i = 0; i < clientfdslength; ++i)
                {
                    if (clientfds[i] != -1 && FD_ISSET(clientfds[i], &readset))
                    {
                        memset(recvbuf, 0, sizeof(recvbuf));
                        //非侦听socket，则接收数据
                        int length = recv(clientfds[i], recvbuf, 64, 0);
                        if (length <= 0 && errno != EINTR)
                        {
                            //收取数据出错了
                            std::cout << "recv data error, clientfd: " << clientfds[i] << std::endl;
                            close(clientfds[i]);
                            //不直接删除该元素，将该位置的元素置位-1
                            clientfds[i] = INVALID_FD;
                            continue;
                        }

                        std::cout << "clientfd: " << clientfds[i] << ", recv data: " << recvbuf << std::endl;
                    }
                }

            }
        }
    }

    //关闭所有客户端socket
    int clientfdslength = clientfds.size();
    for (int i = 0; i < clientfdslength; ++i)
    {
        if (clientfds[i] != INVALID_FD)
        {
            close(clientfds[i]);
        }
    }

    //关闭侦听socket
    close(listenfd);

}

/*
 *在类Linux 系统上一个socket没有建立连接之前，
 *
 */
void test_1()
{
    printf("-----------------------test_1-----------------------\n");
}

int main(int argc, char* argv[])
{
    test_0();
    //test_1();
    return 0;
}