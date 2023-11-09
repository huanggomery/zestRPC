/* 测试EventLoop
 * 首先创建并监听一个门户socket，其回调函数是连接客户请求，并将连接socket加入监听
 * 连接socket的回调函数是读取字符串并写入日志
 * 
 * 需要创建多个线程，其中一个线程作为服务端运行eventloop；其余的线程作为客户端，与服务端建立连接并不断发送字符串
 */
#include "zest/net/eventloop.h"
#include "zest/net/fd_event.h"
#include "zest/common/util.h"
#include "zest/common/logging.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>

using SP_FdEvent = std::shared_ptr<zest::FdEvent>;
using SP_EventLoop = std::shared_ptr<zest::EventLoop>;

const int g_port = 12345;    // 服务器端口
const int N = 4;             // 线程数量
const int max_count = 100;   // 每个线程发送数据条数
SP_EventLoop g_event_loop = nullptr;

void read_and_output(int fd)
{
    char buf[100];
    char *p = buf;
    memset(buf, 0, sizeof(buf));
    int len;
    while ((len = read(fd, buf, 100-(p-buf))) != -1) p += len;
    LOG_DEBUG << buf;
}

void accept_connection(SP_FdEvent fd_event, SP_EventLoop eventloop)
{
    sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    memset(&addr, 0, sizeof(addr));
    int connfd;
    while ((connfd = accept(fd_event->getFd(), (sockaddr*)&addr, &addr_len)) != -1) {
        LOG_INFO << "accept new connection, socket: " << connfd << " ip: " << zest::dotted_decimal_notation(addr) << ":" << zest::src_port(addr);
        zest::set_non_blocking(connfd);

        // 制作connfd的FdEvent，并加入eventloop的监听对象中
        SP_FdEvent connfd_event(new zest::FdEvent(connfd));
        connfd_event->listen(EPOLLIN | EPOLLET, [connfd]() {
            read_and_output(connfd);
        });
        eventloop->addEpollEvent(connfd_event);
    }
}

// 服务器线程，创建eventloop，然后运行loop
void *server_thread_func(void *)
{
    int listenfd = zest::create_socket_and_listen(g_port);
    SP_FdEvent fd_event(new zest::FdEvent(listenfd));
    SP_EventLoop eventloop = zest::EventLoop::CreateEventLoop();
    g_event_loop = eventloop;

    // 门户socket的回调函数就是接受连接
    fd_event->listen(EPOLLIN | EPOLLET, [fd_event, eventloop]() {
        accept_connection(fd_event, eventloop);
    });

    // 让epoll监听listenfd，并开始事件监听循环
    eventloop->addEpollEvent(fd_event);
    eventloop->loop();
    return nullptr;
}

// 客户端线程，向服务器socket发送字节
void *client_thread_func(void *)
{
    int client_fd = zest::create_client_and_connect("127.0.0.1", g_port);
    char buf[20] = {0};
    for (int i = 0; i < max_count; ++i) {
        std::string message = "count = ";
        message += std::to_string(i);
        memset(buf, 0, sizeof(buf));
        strcpy(buf, message.c_str());
        send(client_fd, buf, strlen(buf), 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));   // 防止发送数据太快，服务器接收缓存耗尽
    }
    return nullptr;
}

int main()
{
    zest::Config::SetGlobalConfig("../conf/zest.xml");
    zest::Logger::InitGlobalLogger();
    pthread_t server_tid;
    pthread_t client_tid[N];
    if (pthread_create(&server_tid, NULL, server_thread_func, NULL) != 0) {
        std::cerr << "pthread_create failed" << std::endl;
        exit(-1);
    }
    for (int i = 0; i < N; ++i)
        pthread_create(client_tid+i, NULL, client_thread_func, NULL);
    for (int i = 0; i < N; ++i)
        pthread_join(client_tid[i], NULL);
    
    g_event_loop->stop();
    pthread_join(server_tid, NULL);
    return 0;
}