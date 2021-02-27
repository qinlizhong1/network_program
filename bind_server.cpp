//
// Created by 覃立中 on 2021/2/27.
//g++ -g -o bind_server bind_server.cpp
//

/**
 * TCP服务器通信基本流程
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <vector>

int main(int argc, char* argv[])
{
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
        perror("bind error");
        exit(-1);
    }

    //3.启动侦听
    if (listen(listenfd, SOMAXCONN) == -1)
    {
        perror("listen error");
        exit(-1);
    }

    //记录所有客户端连接的容器
    std::vector<int> clientfds;
    while (true)
    {
        struct sockaddr_in clientaddr;
        bzero(&clientaddr, sizeof(clientaddr));
        socklen_t clientaddrlen = sizeof(clientaddr);
        //4. 接受客户端连接
        int clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddrlen);
        if (clientfd != -1)
        {
            std::cout << "accept success, client ip:port--->"<< inet_ntoa(clientaddr.sin_addr) << ":" << ntohs(clientaddr.sin_port) << std::endl;
            char recvBuf[32] = {0};
            //5. 从客户端接受数据
            int ret = recv(clientfd, recvBuf, 32, 0);
            if (ret > 0)
            {
                std::cout << "recv data from client, data: " << recvBuf << std::endl;
                //6. 将收到的数据原封不动地发给客户端
                ret = send(clientfd, recvBuf, strlen(recvBuf), 0);
                if (ret != strlen(recvBuf))
                    std::cout << "send data error." << std::endl;

                std::cout << "send data to client successfully, data: " << recvBuf << std::endl;
            }
            else
            {
                std::cout << "recv data error." << std::endl;
            }

            //close(clientfd);
            clientfds.push_back(clientfd);
        }
        else
        {
            perror("accept error");
            exit(-1);
        }
    }

    //7.关闭侦听socket
    close(listenfd);

    return 0;
}