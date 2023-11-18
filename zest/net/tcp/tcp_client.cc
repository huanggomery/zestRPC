/* 封装客户端 */
#include "zest/net/tcp/tcp_client.h"
#include "zest/net/rpc/rpc_protocol.h"
#include "zest/net/rpc/rpc_service.h"
#include "zest/net/fd_event.h"
#include "zest/net/timer.h"
#include "zest/common/util.h"
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string>


namespace zest
{

TcpClient::TcpClient(NetAddrBase::s_ptr peer_addr): 
        m_fd(socket(peer_addr->get_family(), SOCK_STREAM, 0)),
        m_local_addr(new IPv4Addr(get_local_addr(m_fd))), m_peer_addr(peer_addr), 
        m_eventloop(EventLoop::CreateEventLoop()), 
        m_connection(new TcpConnection(m_local_addr, m_peer_addr, m_eventloop, m_fd, TcpConnectionByClinet, NotConnected))
{
    if (m_fd == -1) {
        LOG_FATAL << "create socket failed";
        std::cerr << "create socket failed" << std::endl;
        exit(-1);
    }
    if (m_eventloop == nullptr) {
        LOG_FATAL << "create eventloop failed, no more memory available";
        std::cerr << "create eventloop failed, no more memory available" << std::endl;
        exit(-1);
    }
}

void TcpClient::connect()
{
    sockaddr *peer_addr = m_peer_addr->get_sock_addr();
    if (!peer_addr) {
        LOG_ERROR << "Server address is NULL";
        std::cerr << "Server address is NULL" << std::endl;
        return;
    }

    FdEvent::s_ptr fd_event = std::make_shared<FdEvent>(m_fd);
    if (::connect(m_fd, peer_addr, m_peer_addr->get_sock_len()) == -1) {
        // m_fd 是非阻塞的，所以可能会 EINPROGRESS
        if (errno == EINPROGRESS) {
            fd_event->listen(EPOLLOUT, std::bind(&TcpClient::connect, this));
            m_eventloop->addEpollEvent(fd_event);

            bool is_timeout = false;
            auto eventloop = m_eventloop;
            TimerEvent::s_ptr connect_timeout_timer_event = std::make_shared<TimerEvent>(
                1000,
                [&is_timeout, eventloop, fd_event](){
                    is_timeout = true;
                    eventloop->deleteEpollEvent(fd_event);
                    eventloop->stop();
                });
            m_eventloop->addTimerEvent(connect_timeout_timer_event);
            this->start();
            connect_timeout_timer_event->set_valid(false);

            if (is_timeout) {
                std::cerr << "connect timeout" << std::endl;
                LOG_ERROR << "connect timeout";
                ::close(m_fd);
                return;
            }
        }
        else if (errno == EISCONN) {
            /* 表示连接成功，什么都不用做 */
        }
        else {
            LOG_ERROR << "connect to " << m_peer_addr->to_string() << " failed, errno = " << errno;
            std::cerr << "connect to " << m_peer_addr->to_string() << " failed, errno = " << errno << std::endl;
            ::close(m_fd);
            return;
        }
    }

    m_eventloop->deleteEpollEvent(fd_event);
    if (m_eventloop->is_running()) {
        m_eventloop->stop();
    }

    m_connection->set_state(Connected);
}

void TcpClient::prepare(const google::protobuf::Message *request, const std::string &fullname)
{
    m_connection->prepare(request, fullname);
}

void TcpClient::addTimerEvent(TimerEvent::s_ptr timer_event)
{
    m_eventloop->addTimerEvent(timer_event);
}
    

} // namespace zest
