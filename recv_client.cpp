//
// Created by 覃立中 on 2021/2/27.
//g++ -g -o send_client send_client.cpp
//服务端还是使用send_server.cpp用例
//


#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT     3000
#define SEND_DATA       "helloworld"
/*
 * 验证阻塞模式下recv函数的行为：如果服务器端不给客户端发数据，此时客户端调用 recv 函数执行流会阻塞在 recv 函数调用处。
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

    //直接调用recv函数，程序会阻塞在recv函数调用处
    char recvbuf[32] = {0};
    int ret = recv(clientfd, recvbuf, 32, 0);
    if (ret > 0)
    {
        std::cout << "recv successfully." << std::endl;
    }
    else
    {
        std::cout << "recv data error." << std::endl;
    }

    //5. 关闭socket
    close(clientfd);
}

/*
 * socket非阻塞模式下的recv行为:非阻塞模式下如果当前无数据可读，recv 函数将立即返回，返回值为 -1，错误码为 EWOULDBLOCK
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
        char recvbuf[32] = {0};
        int ret = recv(clientfd, recvbuf, 32, 0);
        if (ret > 0)
        {
            //收到了数据
            std::cout << "recv successfully." << std::endl;
        }
        else if (ret == 0)
        {
            //对端关闭了连接
            std::cout << "peer close the socket." << std::endl;
            break;
        }
        else if (ret == -1)
        {
            if (errno == EWOULDBLOCK)
            {
                std::cout << "There is no data available now." << std::endl;
            }
            else if (errno == EINTR)
            {
                //如果被信号中断了，则继续重试recv函数
                std::cout << "recv data interrupted by signal." << std::endl;
            } else
            {
                //真的出错了
                break;
            }
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