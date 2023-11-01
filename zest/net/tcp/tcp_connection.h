/* 封装TCP连接，需要有读写操作以及修改epoll监听事件 */
#ifndef ZEST_NET_TCP_TCP_CONNECTION_H
#define ZEST_NET_TCP_TCP_CONNECTION_H

#include "zest/net/fd_event.h"
#include "zest/net/io_thread.h"
#include "zest/net/eventloop.h"
#include "zest/net/tcp/net_addr.h"
#include "zest/common/noncopyable.h"
#include <memory>
#include <string>

namespace zest
{

class TcpBuffer
{
public:
    TcpBuffer();
    ~TcpBuffer();
    // TODO !!!
private:
    std::string m_buffer;
};


enum TcpConnectionType {
    TcpConnectionByServer = 1,     // 当作服务器使用
    TcpConnectionByClinet = 2,     // 当作客户端使用
};

enum TcpState {
    NotConnected = 1,
    Connected = 2,
    HalfClosing = 3,
    Closed = 4,
};

class TcpConnection: public noncopyable
{
public:
    using s_ptr = std::shared_ptr<TcpConnection>;

    TcpConnection() = delete;
    TcpConnection(NetAddrBase::s_ptr local_addr, NetAddrBase::s_ptr peer_addr, 
                  EventLoop::s_ptr eventloop, int fd, TcpConnectionType type, 
                  TcpState state = NotConnected);

    void listen_read();    // 让套接字监听可读事件
    void listen_write();   // 让套接字监听可写事件
    void clear();

private:
    void tcp_read();
    void tcp_write();
    void execute();    // 读完套接字后，解析、处理请求

private:
    NetAddrBase::s_ptr m_local_addr;
    NetAddrBase::s_ptr m_peer_addr;
    EventLoop::s_ptr m_eventloop;
    int m_fd;
    FdEvent::s_ptr m_fd_event;
    std::string m_in_buffer;
    std::string m_out_buffer;
    TcpConnectionType m_connection_type;
    TcpState m_state;
};
    
} // namespace zest


#endif