/* 封装TCP连接，需要有读写操作以及修改epoll监听事件 */
#include "zest/net/tcp/tcp_connection.h"
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>


namespace zest
{

TcpBuffer::TcpBuffer()
{
    // TODO !!!

}

TcpBuffer::~TcpBuffer()
{
    // TODO !!!

}
    
TcpConnection::TcpConnection(NetAddrBase::s_ptr local_addr, NetAddrBase::s_ptr peer_addr, 
                             EventLoop::s_ptr eventloop, int fd, TcpConnectionType type, 
                             TcpState state /* =NotConnected */)
    : m_local_addr(local_addr), m_peer_addr(peer_addr), 
      m_eventloop(eventloop), m_fd(fd), m_fd_event(new FdEvent(fd)),
      m_connection_type(type), m_state(state)
{
    m_fd_event->set_non_blocking();
    if (m_connection_type == TcpConnectionByServer) {
        listen_read();
    }
}

// 让套接字监听可读事件
void TcpConnection::listen_read()
{
    m_fd_event->listen(EPOLLIN | EPOLLET | EPOLLONESHOT, std::bind(TcpConnection::tcp_read, this));
    m_eventloop->addEpollEvent(m_fd_event);
}

// 让套接字监听可写事件
void TcpConnection::listen_write()
{
    m_fd_event->listen(EPOLLOUT | EPOLLET | EPOLLONESHOT, std::bind(TcpConnection::tcp_write, this));
    m_eventloop->addEpollEvent(m_fd_event);
}

void TcpConnection::clear()
{
    if (m_state == Closed)
        return;
    m_eventloop->deleteEpollEvent(m_fd_event);
    m_state = Closed;
}

void TcpConnection::tcp_read()
{
    bool is_error = false, is_closed = false;
    char tmp_buf[1024];

    while (true) {
        memset(tmp_buf, 0, sizeof(tmp_buf));
        int len = recv(m_fd, tmp_buf, sizeof(tmp_buf), 0);
        if (len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else if (errno == EINTR)
                continue;
            else {
                is_error = true;
                break;
            }
        }
        else if (len == 0) {
            is_closed = true;
            break;
        }
        // 正常情况
        else {
            m_in_buffer += tmp_buf;
        }
    }

    if (is_error) {
        // TODO
        return;
    }
    if (is_closed) {
        // TODO
        return;
    }

    execute();
}

void TcpConnection::tcp_write()
{
    bool is_error = false;

    while (!m_out_buffer.empty()) {
        const char *ptr = m_out_buffer.c_str();
        int len = send(m_fd, ptr, m_out_buffer.size(), 0);
        if (len == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else if (errno == EINTR)
                continue;
            else {
                is_error = true;
                break;
            }
        }
        m_out_buffer = m_out_buffer.substr(len);
    }

    if (is_error) {
        // TODO
        return;
    }
}

void TcpConnection::execute()
{
    // TODO
    // 目前只是简单地echo

    m_out_buffer.swap(m_in_buffer);
    listen_write();
}

} // namespace zest
