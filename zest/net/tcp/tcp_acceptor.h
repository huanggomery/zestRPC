/* 对网络监听套接字的封装 */
#ifndef ZEST_NET_TCP_TCP_ACCEPTOR_H
#define ZEST_NET_TCP_TCP_ACCEPTOR_H
#include "zest/common/noncopyable.h"
#include "zest/net/tcp/net_addr.h"
#include <unistd.h>
#include <memory>
#include <unordered_map>


namespace zest
{
    
class TcpAcceptor : public noncopyable
{
public:
    using s_ptr = std::shared_ptr<TcpAcceptor>;
    
    TcpAcceptor() = delete;
    TcpAcceptor(NetAddrBase::s_ptr addr);
    ~TcpAcceptor() { close(m_listenfd); }
    int get_listenfd() const {return m_listenfd;}
    std::unordered_map<int, NetAddrBase::s_ptr> accept();
    
private:
    NetAddrBase::s_ptr m_addr {nullptr};
    int m_domain;
    int m_listenfd {-1};
};
    
} // namespace zest

#endif