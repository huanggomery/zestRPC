/* 封装TCP连接，需要有读写操作、业务处理以及修改epoll监听事件 */
#ifndef ZEST_NET_TCP_TCP_CONNECTION_H
#define ZEST_NET_TCP_TCP_CONNECTION_H

#include "zest/net/fd_event.h"
#include "zest/net/io_thread.h"
#include "zest/net/eventloop.h"
#include "zest/net/tcp/net_addr.h"
#include "zest/net/tcp/tcp_buffer.h"
#include "zest/net/rpc/rpc_protocol.h"
#include "zest/common/noncopyable.h"
#include <google/protobuf/message.h>
#include <memory>
#include <string>

namespace zest
{

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

class TcpConnection: public noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    using s_ptr = std::shared_ptr<TcpConnection>;

    TcpConnection() = delete;
    TcpConnection(NetAddrBase::s_ptr local_addr, NetAddrBase::s_ptr peer_addr, 
                  EventLoop::s_ptr eventloop, int fd, TcpConnectionType type, 
                  TcpState state = NotConnected);
                  
    ~TcpConnection();

    void listen_read();    // 让套接字监听可读事件
    void listen_write();   // 让套接字监听可写事件
    void clear();

    // 获取发送缓存
    TcpBuffer& out_buffer() {return m_out_buffer;}

    // 设置连接状态
    void set_state(TcpState state) {m_state = state;}

    // 返回连接状态
    TcpState get_state() const {return m_state;}

    // 填充各个字段，然后将协议编码成字节序列后放入发送缓存，等待可写
    void prepare(const google::protobuf::Message *request, const std::string &fullname);

    // 得到从服务器返回的协议，仅用于客户端
    RpcProtocol::s_ptr get_rsp_protocol() const {return m_rsp_protocol;}

private:
    void tcp_read();
    void tcp_write();
    void server_execute();    // 读完套接字后，解析、处理请求
    void client_execute();    // 读完套接字后，解析返回的协议
    void shutdown();   // 半关闭

private:
    NetAddrBase::s_ptr m_local_addr;
    NetAddrBase::s_ptr m_peer_addr;
    EventLoop::s_ptr m_eventloop;
    int m_fd;
    FdEvent::s_ptr m_fd_event;
    TcpBuffer m_in_buffer;
    TcpBuffer m_out_buffer;
    RpcDecoder m_decoder;
    TcpConnectionType m_connection_type;
    TcpState m_state;
    RpcProtocol::s_ptr m_rsp_protocol;   // 用于客户端，保存从服务器返回的协议
};
    
} // namespace zest


#endif