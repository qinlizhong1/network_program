//
// Created by 覃立中 on 2021/2/27.
//http://balloonwj.gitee.io/cpp-guide-web/articles/%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B/select%E5%87%BD%E6%95%B0%E9%87%8D%E9%9A%BE%E7%82%B9%E8%A7%A3%E6%9E%90.html
//g++ -g -o select_client select_client.cpp
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

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT     3000
/*
 */
void test_0()
{
    printf("-----------------------test_0-----------------------\n");

    //创建一个socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        perror("socket error");
        exit(-1);
    }

    //连接服务器
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    serveraddr.sin_port = htons(SERVER_PORT);
    if (connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        close(clientfd);
        perror("connect error");
        exit(-1);
    }

    fd_set readset;
    FD_ZERO(&readset);

    //将侦听socket加入到待检测的可读事件中去
    FD_SET(clientfd, &readset);
    timeval tm;
    tm.tv_sec = 5;
    tm.tv_usec = 0;
    int ret;
    int count = 0;
    fd_set backup_readset;
    memcpy(&backup_readset, &readset, sizeof(fd_set));
    while (true)
    {
        if (memcmp(&readset, &backup_readset, sizeof(fd_set)) == 0)
        {
            std::cout << "equal" << std::endl;
        }
        else
        {
            std::cout << "not equal" << std::endl;
        }

        //暂且只检测可读事件，不检测可写和异常事件
        ret = select(clientfd + 1, &readset, NULL, NULL, &tm);
        std::cout << "tm.tv_sec: " << tm.tv_sec << ", tm.tv_usec: " << tm.tv_usec << std::endl;
        if (ret == -1)
        {
            //除了被信号中断的情形，其他情况都是出错
            if (errno != EINTR)
                break;
        } else if (ret == 0){
            //select函数超时
            std::cout << "no event in specific time interval, count：" << count << std::endl;
            ++count;
            continue;
        } else {
            if (FD_ISSET(clientfd, &readset))
            {
                //检测到可读事件
                char recvbuf[32];
                memset(recvbuf, 0, sizeof(recvbuf));
                //假设对端发数据的时候不超过31个字符。
                int n = recv(clientfd, recvbuf, 32, 0);
                if (n < 0)
                {
                    //除了被信号中断的情形，其他情况都是出错
                    if (errno != EINTR)
                        break;
                } else if (n == 0) {
                    //对端关闭了连接
                    break;
                } else {
                    std::cout << "recv data: " << recvbuf << std::endl;
                }
            }
            else
            {
                std::cout << "other socket event." << std::endl;
            }
        }
    }

    //关闭socket
    close(clientfd);
}

/*
 *test_0版本有缺陷：如果复用这些 fd_set 变量，必须按上文所说的重新清零再重新添加关心的 socket 到集合中去。
 *
 */
void test_1()
{
    printf("-----------------------test_1-----------------------\n");
    //创建一个socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        perror("socket error");
        exit(-1);
    }

    //连接服务器
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    serveraddr.sin_port = htons(SERVER_PORT);
    if (connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        perror("connect error");
        close(clientfd);
        exit(-1);
    }

    int ret;
    while (true)
    {
        fd_set readset;
        FD_ZERO(&readset);
        //将侦听socket加入到待检测的可读事件中去
        FD_SET(clientfd, &readset);
        timeval tm;
        tm.tv_sec = 0;
        tm.tv_usec = 0;

        //暂且只检测可读事件，不检测可写和异常事件
        ret = select(clientfd + 1, &readset, NULL, NULL, &tm);
        std::cout << "tm.tv_sec: " << tm.tv_sec << ", tm.tv_usec: " << tm.tv_usec << std::endl;
        if (ret == -1)
        {
            //除了被信号中断的情形，其他情况都是出错
            if (errno != EINTR)
                break;
        } else if (ret == 0){
            //select函数超时
            std::cout << "no event in specific time interval." << std::endl;
            continue;
        } else {
            if (FD_ISSET(clientfd, &readset))
            {
                //检测到可读事件
                char recvbuf[32];
                memset(recvbuf, 0, sizeof(recvbuf));
                //假设对端发数据的时候不超过31个字符。
                int n = recv(clientfd, recvbuf, 32, 0);
                if (n < 0)
                {
                    //除了被信号中断的情形，其他情况都是出错
                    if (errno != EINTR)
                        break;
                } else if (n == 0) {
                    //对端关闭了连接
                    break;
                } else {
                    std::cout << "recv data: " << recvbuf << std::endl;
                }
            }
            else
            {
                std::cout << "other socket event." << std::endl;
            }
        }
    }


    //关闭socket
    close(clientfd);

}

int main(int argc, char* argv[])
{
    //test_0();
    test_1();
    return 0;
}