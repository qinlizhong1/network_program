//
// Created by 覃立中 on 2021/2/27.
//http://balloonwj.gitee.io/cpp-guide-web/articles/%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B/connect%E5%87%BD%E6%95%B0%E5%9C%A8%E9%98%BB%E5%A1%9E%E5%92%8C%E9%9D%9E%E9%98%BB%E5%A1%9E%E6%A8%A1%E5%BC%8F%E4%B8%8B%E7%9A%84%E8%A1%8C%E4%B8%BA.html
//g++ -g -o connect connect.cpp
//

/*
 * 异步的connect技术（或者叫非阻塞的 connect）解决的问题：在 socket 是阻塞模式下 connect 函数会一直到有明确的结果才会返回（或连接成功或连接失败），
   如果服务器地址“较远”，连接速度比较慢，connect 函数在连接过程中可能会导致程序阻塞在connect函数处好一会儿（如两三秒之久）

 *异步connetc流程：
    1. 创建socket，并将 socket 设置成非阻塞模式；
    2. 调用 connect 函数，此时无论 connect 函数是否连接成功会立即返回；如果返回-1并不表示连接出错，如果此时错误码是EINPROGRESS
    3. 接着调用 select 函数，在指定的时间内判断该 socket 是否可写，如果可写说明连接成功，反之则认为连接失败。
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT     3000
#define SEND_DATA       "helloworld, lizhong"

/*
 *在类Linux 系统上一个socket没有建立连接之前，
 *
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

    //连接成功以后，我们再将 clientfd 设置成非阻塞模式，
    //不能在创建时就设置，这样会影响到 connect 函数的行为
    int oldSocketFlag = fcntl(clientfd, F_GETFL, 0);
    int newSocketFlag = oldSocketFlag | O_NONBLOCK;
    if (fcntl(clientfd, F_SETFL,  newSocketFlag) == -1)
    {
        close(clientfd);
        perror("fcntl error");
        exit(-1);
    }

    //2.连接服务器
    struct sockaddr_in serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    serveraddr.sin_port = htons(SERVER_PORT);
    for (;;)
    {
        int ret = connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
        if (ret == 0)
        {
            std::cout << "connect to server successfully." << std::endl;
            close(clientfd);
            return;
        }
        else if (ret == -1)
        {
            if (errno == EINTR)
            {
                //connect 动作被信号中断，重试connect
                std::cout << "connecting interruptted by signal, try again." << std::endl;
                continue;
            } else if (errno == EINPROGRESS)
            {
                //连接正在尝试中
                break;
            } else {
                //真的出错了，
                close(clientfd);
                perror("connect error");
                exit(-1);
            }
        }
    }

    fd_set writeset;
    FD_ZERO(&writeset);
    FD_SET(clientfd, &writeset);
    //可以利用tv_sec和tv_usec做更小精度的超时控制
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    if (select(clientfd + 1, NULL, &writeset, NULL, &tv) == 1)
    {
        std::cout << "[select] connect to server successfully." << std::endl;
    } else {
        std::cout << "[select] connect to server error." << std::endl;
    }

    //5. 关闭socket
    close(clientfd);
}

/*
 *在类Linux 系统上一个 socket 没有建立连接之前，用 select 函数检测其是否可写，你也会得到可写得结果，
  所以上述流程并不适用于 Linux 系统。正确的做法是，connect 之后，不仅要用 select 检测可写，还要检测此时 socket 是否出错，
  通过错误码来检测确定是否连接上，错误码为 0 表示连接上，反之为未连接上。
 */
void test_1()
{
    printf("-----------------------test_1-----------------------\n");
    //1.创建一个socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        perror("socket error");
        exit(-1);
    }

    //连接成功以后，我们再将 clientfd 设置成非阻塞模式，
    //不能在创建时就设置，这样会影响到 connect 函数的行为
    int oldSocketFlag = fcntl(clientfd, F_GETFL, 0);
    int newSocketFlag = oldSocketFlag | O_NONBLOCK;
    if (fcntl(clientfd, F_SETFL,  newSocketFlag) == -1)
    {
        close(clientfd);
        perror("fcntl error");
        exit(-1);
    }

    //2.连接服务器
    struct sockaddr_in serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    serveraddr.sin_port = htons(SERVER_PORT);
    for (;;)
    {
        int ret = connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
        if (ret == 0)
        {
            std::cout << "connect to server successfully." << std::endl;
            close(clientfd);
            return;
        }
        else if (ret == -1)
        {
            if (errno == EINTR)
            {
                //connect 动作被信号中断，重试connect
                std::cout << "connecting interruptted by signal, try again." << std::endl;
                continue;
            } else if (errno == EINPROGRESS)
            {
                //连接正在尝试中
                break;
            } else {
                //真的出错了，
                close(clientfd);
                perror("connect error");
                exit(-1);
            }
        }
    }

    fd_set writeset;
    FD_ZERO(&writeset);
    FD_SET(clientfd, &writeset);
    //可以利用tv_sec和tv_usec做更小精度的超时控制
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    if (select(clientfd + 1, NULL, &writeset, NULL, &tv) != 1)
    {
        std::cout << "[select] connect to server error." << std::endl;
        exit(-1);
    }

    int err;
    socklen_t len = static_cast<socklen_t>(sizeof err);
    if (::getsockopt(clientfd, SOL_SOCKET, SO_ERROR, &err, &len) < 0)
    {
        close(clientfd);
        perror("getsockopt error");
        exit(-1);
    }

    if (err == 0)
        std::cout << "connect to server successfully." << std::endl;
    else
        std::cout << "connect to server error." << std::endl;

    //5. 关闭socket
    close(clientfd);
}

int main(int argc, char* argv[])
{
    //test_0();
    test_1();
    return 0;
}