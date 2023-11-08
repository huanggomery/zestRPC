/* 封装TCP连接，需要有读写操作以及修改epoll监听事件 */
#include "zest/net/tcp/tcp_connection.h"
#include "zest/net/tcp/tcp_server.h"
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

namespace zest
{
    
TcpConnection::TcpConnection(NetAddrBase::s_ptr local_addr, NetAddrBase::s_ptr peer_addr, 
                             EventLoop::s_ptr eventloop, int fd, TcpConnectionType type, 
                             TcpState state /* =NotConnected */)
    : m_local_addr(local_addr), m_peer_addr(peer_addr), 
      m_eventloop(eventloop), m_fd(fd), m_fd_event(new FdEvent(fd)),
      m_in_buffer(), m_out_buffer(), m_decoder(m_in_buffer),
      m_connection_type(type), m_state(state)
{
    m_fd_event->set_non_blocking();
    if (m_connection_type == TcpConnectionByServer) {
        listen_read();
    }
}

TcpConnection::~TcpConnection()
{
    clear();
}

// 让套接字监听可读事件
void TcpConnection::listen_read()
{
    m_fd_event->listen(EPOLLIN | EPOLLET | EPOLLONESHOT, std::bind(&TcpConnection::tcp_read, this));
    m_eventloop->addEpollEvent(m_fd_event);
}

// 让套接字监听可写事件
void TcpConnection::listen_write()
{
    m_fd_event->listen(EPOLLOUT | EPOLLET | EPOLLONESHOT, std::bind(&TcpConnection::tcp_write, this));
    m_eventloop->addEpollEvent(m_fd_event);
}

void TcpConnection::clear()
{
    if (m_state == Closed)
        return;
    ::close(m_fd);
    m_eventloop->deleteEpollEvent(m_fd_event);
    m_state = Closed;
    LOG_INFO << "TCP disconnection, peer address: " << m_peer_addr->to_string();
}

void TcpConnection::tcp_read()
{
    bool is_error = false, is_closed = false, is_finished = false;
    char tmp_buf[1024];

    if (m_state == HalfClosing)
        is_closed = true;

    while (!is_error && !is_closed && !is_finished) {
        memset(tmp_buf, 0, sizeof(tmp_buf));
        int len = recv(m_fd, tmp_buf, sizeof(tmp_buf), 0);
        if (len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                is_finished = true;
            else if (errno == EINTR)
                continue;
            else {
                is_error = true;
            }
        }
        else if (len == 0) {
            is_closed = true;
        }
        // 正常情况
        else {
            m_in_buffer += tmp_buf;
        }
    }

    // 出错的情况，半关闭连接，然后等待对端关闭
    if (is_error) {
        shutdown();
        listen_read();
        LOG_ERROR << "TCP read error, shutdown connection";
        return;
    }
    if (is_closed) {
        TcpServer::GetTcpServer()->remove_connection(m_fd);  // 随后会马上调用析构函数
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
        m_out_buffer.move_forward(len);
    }

    if (is_error) {
        shutdown();
        LOG_ERROR << "TCP write error, shutdown connection";
    }

    listen_read();
}

void TcpConnection::execute()
{
    RpcProtocol::s_ptr req_protocol;
    while (req_protocol = m_decoder.decode()) {
        // TODO: 处理RPC请求。并将结果写入 m_out_buffer
    }

    // 判断RPC请求是否出错
    if (m_decoder.is_failed()) {
        // TODO: 收到错误的请求，应该发送带有错误码和错误信息的RPC响应
    }

    // m_out_buffer 非空，则等待发送
    if (!m_out_buffer.empty()) {
        listen_write();
    }
}

void TcpConnection::shutdown()
{
    if (m_state == Closed || m_state == NotConnected)
        return;
    m_state = HalfClosing;
    ::shutdown(m_fd, SHUT_WR);
}

} // namespace zest
