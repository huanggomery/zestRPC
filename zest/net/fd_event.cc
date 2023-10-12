/* epoll监听事件的封装 */
#include "zest/net/fd_event.h"
#include "zest/common/util.h"
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

namespace zest
{

FdEvent::FdEvent(int fd): m_fd(fd)
{
    memset(&m_event, 0, sizeof(m_event));
}

// 为IO事件设置回调函数
void FdEvent::listen(uint32_t ev_type, CallBackFunc cb, CallBackFunc err_cb /*=nullptr*/)
{
    m_event.events = ev_type;
    if (ev_type & EPOLLIN)
        m_read_callback = cb;
    else
        m_write_callback = cb;
    if (err_cb)
        m_error_callback = err_cb;
    m_event.data.fd = m_fd;
}

// 获取IO事件的回调函数
std::function<void()> FdEvent::handler(TriggerEvent type) const
{
    if (type == IN_EVENT)
        return m_read_callback;
    else if (type == OUT_EVENT)
        return m_write_callback;
    else if (type == ERROR_EVENT)
        return m_error_callback;
    else
        return nullptr;
}

// 将监听的fd设置为非阻塞
void FdEvent::set_non_blocking()
{
    zest::set_non_blocking(m_fd);   // 使用的是utils.h中定义的函数
}

// 向fd中写入一个字节，用于从epoll_wait中返回
void WakeUpFdEvent::wakeup()
{
    char buf[8] = {'a'};
    int rt = write(m_fd, buf, 8);
    if (rt != 8) {
        LOG_ERROR << "Write to wakeup fd " << m_fd << " failed, errno = " << errno;
    }
    else {
        // LOG_DEBUG << "Write to wakeup fd " << m_fd << " succeed";
    }
}

} // namespace zest
