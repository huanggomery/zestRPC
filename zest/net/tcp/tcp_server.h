/* 主线程，拥有tcp_acceptor、线程池以及自己的eventloop */
#ifndef ZEST_NET_TCP_TCP_SERVER_H
#define ZEST_NET_TCP_TCP_SERVER_H
#include "zest/common/noncopyable.h"
#include "zest/net/tcp/tcp_acceptor.h"
#include "zest/net/tcp/tcp_connection.h"
#include "zest/net/io_thread.h"
#include "zest/net/eventloop.h"
#include "zest/net/tcp/net_addr.h"
#include <memory>
#include <unordered_map>


namespace zest
{

class TcpServer : public noncopyable
{
public:
    using s_ptr = std::shared_ptr<TcpServer>;

    TcpServer(NetAddrBase::s_ptr local_addr);
    ~TcpServer();

    // 服务器，启动！
    void start();

    // 关闭服务器
    void stop();

    // 本地监听套接字的回调函数
    void accept_callback();   

private:
    NetAddrBase::s_ptr m_local_addr;       // 监听的本地地址
    TcpAcceptor::s_ptr m_acceptor;         // TCP连接接收器
    EventLoop::s_ptr m_main_eventloop;     // 主线程eventloop，负责监听本地地址的套接字
    ThreadPool::s_ptr m_thread_pool;       // 线程池
    bool m_running {false};                // 服务器是否运行的标志
    std::unordered_map<int, TcpConnection::s_ptr> m_connections;   // 所有的TCP连接
};
    
} // namespace zest


#endif