/* 对网络监听套接字的封装 */
#include "zest/net/tcp/tcp_acceptor.h"
#include "zest/common/logging.h"
#include "zest/common/util.h"
#include <sys/socket.h>
#include <errno.h>

namespace zest
{

// 构造函数，创建监听套接字，然后绑定地址，开始监听
TcpAcceptor::TcpAcceptor(NetAddrBase::s_ptr addr) : 
    m_addr(addr), m_domain(addr->get_family())
{
    // 检查地址是否合法
    if (!addr->check()) {
        LOG_FATAL << "invalid address: " << addr->to_string();
        exit(-1);   // 连监听套接字都没有，服务器完全不能使用，因此直接退出
    }

    m_listenfd = socket(addr->get_family(), SOCK_STREAM, 0);
    if (m_listenfd == -1) {
        LOG_FATAL << "socket failed";
        exit(-1);
    }

    set_non_blocking(m_listenfd);

    // 终止time-wait
    int reuse = 1;
    if (setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        LOG_ERROR << "setsockopt REUSEADDR failed, errno = " << errno;
    }

    socklen_t len = addr->get_sock_len();
    if (bind(m_listenfd, addr->get_sock_addr(), len) == -1) {
        LOG_FATAL << "bind failed, errno = " << errno;
        close(m_listenfd);
        exit(-1);
    }

    if (listen(m_listenfd, 1000) == -1) {
        LOG_FATAL << "listen failed, errno = " << errno;
        close(m_listenfd);
        exit(-1);
    }
    
    LOG_DEBUG << "create listenfd successful";
}

int TcpAcceptor::accept()
{
    if (m_domain == PF_INET) {
        sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        memset(&client_addr, 0, len);
        int clientfd = ::accept(m_listenfd, reinterpret_cast<sockaddr*>(&client_addr), &len);
        if (clientfd == -1) {
            LOG_ERROR << "accept failed, errno = " << errno;
        }
        return clientfd;
    }
    else {
        // other protocol...
        return -1;
    }
}

} // namespace zest
