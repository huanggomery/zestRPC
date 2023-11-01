/* 主线程，拥有tcp_acceptor、线程池以及自己的eventloop */
#include "zest/net/tcp/tcp_server.h"
#include "zest/net/fd_event.h"
#include "zest/common/config.h"

namespace zest
{

TcpServer::TcpServer(NetAddrBase::s_ptr local_addr) :
    m_local_addr(local_addr), 
    m_acceptor(new TcpAcceptor(local_addr)),
    m_main_eventloop(EventLoop::CreateEventLoop()),
    m_thread_pool(new ThreadPool(Config::GetGlobalConfig()->io_threads()))
{
    if (!local_addr) {
        LOG_FATAL << "argument error, get nullptr address";
        exit(-1);
    }
    if (!m_acceptor) {
        LOG_FATAL << "Create TcpAcceptor failed";
        exit(-1);
    }
    if (!m_main_eventloop) {
        LOG_FATAL << "Create main eventloop failed";
        exit(-1);
    }
    if (!m_thread_pool) {
        LOG_FATAL << "Create thread pool failed";
        exit(-1);
    }

    FdEvent::s_ptr listenfd_event(new FdEvent(m_acceptor->get_listenfd()));
    listenfd_event->listen(EPOLLIN | EPOLLET, std::bind(accept_callback, this));
    m_main_eventloop->addEpollEvent(listenfd_event);
}

// 服务器，启动！
void TcpServer::start()
{
    TODO!!!
}

// 本地监听套接字的回调函数
void TcpServer::accept_callback()
{
    TODO!!!
}

} // namespace zest