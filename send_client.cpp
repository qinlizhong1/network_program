//
// Created by 覃立中 on 2021/2/27.
//g++ -g -o send_client send_client.cpp
//

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <fcntl.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT     3000
#define SEND_DATA       "helloworld"
/*
 * socket 阻塞模式下的 send 行为
 */
void test_0()
{
    printf("-----------------------test_0-----------------------\n");

    //1.创建一个socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        close(clientfd);
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

    //3. 不断向服务器发送数据，或者出错退出
    int count = 0;
    while (true)
    {
        int ret = send(clientfd, SEND_DATA, strlen(SEND_DATA), 0);
        if (ret != strlen(SEND_DATA))
        {
            std::cout << "send data error." << std::endl;
            break;
        }
        else
        {
            count ++;
            std::cout << "send data successfully, count = " << count << std::endl;
        }
    }

    //5. 关闭socket
    close(clientfd);
}

/*
 * socket非阻塞模式下的 send 行为
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

    //2.连接服务器
    struct sockaddr_in serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    serveraddr.sin_port = htons(SERVER_PORT);
    if (connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        close(clientfd);
        perror("connect error");
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

    //3. 不断向服务器发送数据，或者出错退出
    int count = 0;
    while (true)
    {
        int ret = send(clientfd, SEND_DATA, strlen(SEND_DATA), 0);
        if (ret == -1)
        {
            //非阻塞模式下send函数由于TCP窗口太小发不出去数据，错误码是EWOULDBLOCK
            if (errno == EWOULDBLOCK)
            {
                std::cout << "send data error as TCP Window size is too small." << std::endl;
                continue;
            }
            else if (errno == EINTR)
            {
                //如果被信号中断，我们继续重试
                std::cout << "sending data interrupted by signal." << std::endl;
                continue;
            }
            else
            {
                std::cout << "send data error." << std::endl;
                break;
            }
        }
        if (ret == 0)
        {
            //对端关闭了连接，我们也关闭
            std::cout << "send data error." << std::endl;
            close(clientfd);
            break;
        }
        else
        {
            count ++;
            std::cout << "send data successfully, count = " << count << std::endl;
        }
    }

    //5. 关闭socket
    close(clientfd);

}

int main(int argc, char* argv[])
{
    //test_0();
    test_1();
    return 0;
}