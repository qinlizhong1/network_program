//
// Created by 覃立中 on 2021/2/27.
//

/**
 * TCP客户端通信基本流程
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT     3000
#define SEND_DATA       "helloworld,lizhong"


/*
 * 客户端不绑定端口，操作系统自动分配
 */
void test_0()
{
    printf("-----------------------test_0-----------------------\n");
    //1.创建一个socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        perror("socket error");
        exit(-1);
    }

    //2.连接服务器
    struct sockaddr_in serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    serveraddr.sin_port = htons(SERVER_PORT);
    if (connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        perror("connect error");
        exit(-1);
    }

    //3. 向服务器发送数据
    int ret = send(clientfd, SEND_DATA, strlen(SEND_DATA), 0);
    if (ret != strlen(SEND_DATA))
    {
        std::cout << "send data error." << std::endl;
        exit(-1);
    }

    std::cout << "send data successfully, data: " << SEND_DATA << std::endl;

    //4. 从客户端收取数据
    char recvBuf[32] = {0};
    ret = recv(clientfd, recvBuf, 32, 0);
    if (ret > 0)
    {
        std::cout << "recv data successfully, data: " << recvBuf << std::endl;
    }
    else
    {
        std::cout << "recv data error, data: " << recvBuf << std::endl;
    }

    //5. 关闭socket
    //close(clientfd);
    //这里仅仅是为了让客户端程序不退出
    while (true)
    {
        sleep(3);
    }
}

/*
 * 客户端绑定端口
 */
void test_1()
{
    printf("\n-----------------------test_1-----------------------\n");
    //1.创建一个socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        perror("socket error");
        exit(-1);
    }

    //将socket绑定到20000号端口上去
    struct sockaddr_in bindaddr;
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bindaddr.sin_port = htons(20000);
    if (bind(clientfd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) == -1)
    {
        perror("bind error");
        exit(-1);
    }

    //2.连接服务器
    struct sockaddr_in serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    serveraddr.sin_port = htons(SERVER_PORT);
    if (connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        perror("connect error");
        exit(-1);
    }

    //3. 向服务器发送数据
    int ret = send(clientfd, SEND_DATA, strlen(SEND_DATA), 0);
    if (ret != strlen(SEND_DATA))
    {
        std::cout << "send data error." << std::endl;
        exit(-1);
    }

    std::cout << "send data successfully, data: " << SEND_DATA << std::endl;

    //4. 从客户端收取数据
    char recvBuf[32] = {0};
    ret = recv(clientfd, recvBuf, 32, 0);
    if (ret > 0)
    {
        std::cout << "recv data successfully, data: " << recvBuf << std::endl;
    }
    else
    {
        std::cout << "recv data error, data: " << recvBuf << std::endl;
    }

    //5. 关闭socket
    //close(clientfd);
    //这里仅仅是为了让客户端程序不退出
    while (true)
    {
        sleep(3);
    }
}

int main(int argc, char* argv[])
{
    //test_0();
    test_1();
    return 0;
}