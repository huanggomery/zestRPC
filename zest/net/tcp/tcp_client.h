/* 封装客户端 */
#ifndef ZEST_NET_TCP_TCP_CLIENT_H
#define ZEST_NET_TCP_TCP_CLIENT_H
#include "zest/common/noncopyable.h"
#include "zest/net/tcp/tcp_connection.h"
#include "zest/net/tcp/net_addr.h"
#include "zest/net/eventloop.h"
#include <google/protobuf/message.h>
#include <memory>


namespace zest
{

class TcpClient: public noncopyable
{
public:
    using s_ptr = std::shared_ptr<TcpClient>;

    TcpClient() = delete;

    TcpClient(NetAddrBase::s_ptr peer_addr);

    void connect();

    void prepare(const google::protobuf::Message *request, const std::string &fullname);

    void stop() {m_eventloop->stop();}

    void start() {m_eventloop->loop();}

    TcpState get_state() const {return m_connection->get_state();}

    RpcProtocol::s_ptr get_rsp_protocol() const {return m_connection->get_rsp_protocol();}

    // 添加一个定时器
    void addTimerEvent(TimerEvent::s_ptr timer_event);

private:
    int m_fd;
    NetAddrBase::s_ptr m_local_addr;
    NetAddrBase::s_ptr m_peer_addr;
    EventLoop::s_ptr m_eventloop;
    TcpConnection::s_ptr m_connection;
};
    
} // namespace zest



#endif