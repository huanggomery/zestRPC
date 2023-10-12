/* epoll监听IO事件的封装 */
#ifndef ZEST_NET_FD_EVENT_H
#define ZEST_NET_FD_EVENT_H
#include <sys/epoll.h>
#include <functional>
#include "zest/common/noncopyable.h"
#include "zest/common/logging.h"

namespace zest
{

// IO事件类的基类
class FdEvent: public noncopyable
{
public:
    using CallBackFunc = std::function<void()>;
    enum TriggerEvent {
        IN_EVENT = EPOLLIN,
        OUT_EVENT = EPOLLOUT,
        ERROR_EVENT = EPOLLERR
    };

public:
    FdEvent() = delete;
    ~FdEvent() = default;
    FdEvent(int fd);

    // 为IO事件设置回调函数
    void listen(uint32_t ev_type, CallBackFunc cb, CallBackFunc err_cb = nullptr);

    // 获取IO事件的回调函数
    CallBackFunc handler(TriggerEvent type) const;

    // 获取文件描述符
    int getFd() const {return m_fd;}

    // 获取epoll_event结构体
    epoll_event getEpollEvent() const {return m_event;}

    // 将监听的fd设置为非阻塞
    void set_non_blocking();
    
protected:
    int m_fd {-1};
    struct epoll_event m_event;
    CallBackFunc m_read_callback {nullptr};
    CallBackFunc m_write_callback {nullptr};
    CallBackFunc m_error_callback {nullptr};
};


class WakeUpFdEvent: public FdEvent
{
public:
    WakeUpFdEvent() = delete;
    WakeUpFdEvent(int fd): FdEvent(fd) { /* do nothing else */}
    ~WakeUpFdEvent() = default;

    void wakeup();  // 向fd中写入一个字节，用于从epoll_wait中返回

private:
    /* Nothing but members inherit from Base Class FdEvent */
};

    
} // namespace zest


#endif