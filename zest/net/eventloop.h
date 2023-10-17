/* Reactor模型 */
#ifndef ZEST_NET_EVENT_LOOP_H
#define ZEST_NET_EVENT_LOOP_H
#include "zest/common/sync.h"
#include "zest/common/noncopyable.h"
#include "zest/net/fd_event.h"
#include "zest/net/timer.h"
#include <functional>
#include <queue>
#include <unordered_map>
#include <pthread.h>
#include <memory>

namespace zest
{

class EventLoop: public noncopyable
{
public:
    using s_ptr = std::shared_ptr<EventLoop>;
    using CallBackFunc = std::function<void()>;  // 回调函数
    using SP_FdEvent = std::shared_ptr<FdEvent>;
    using SP_TimerEvent = std::shared_ptr<TimerEvent>;
public:
    static std::shared_ptr<EventLoop> CreateEventLoop();   // 工厂函数
    ~EventLoop() = default;

    // 核心功能：循环监听注册在epoll_fd上的文件描述符，并处理回调函数
    void loop();

    // 停止loop循环
    void stop();

    // 唤醒epoll_wait
    void wakeup() {m_wakeup_event->wakeup();}

    void addEpollEvent(SP_FdEvent fd_event);
    void deleteEpollEvent(SP_FdEvent fd_event);

    // 添加一个定时器
    void addTimerEvent(SP_TimerEvent timer_event);
    
private:
    EventLoop();
    void addTask(CallBackFunc cb, bool wake_up = false);
    bool isThisThread() const;   // 判断调用者是否是创建该对象的线程

private:
    std::unordered_map<int, SP_FdEvent> m_listen_fds;  // 所有监听的fd的集合
    pid_t m_tid {0};                        // 记录创建该对象的线程号
    int m_epoll_fd {0};                         // epoll_fd
    bool m_stop_flag {false};                   // 是否停止
    std::queue<CallBackFunc> m_pending_tasks;   // 等待处理的回调函数
    Mutex m_mutex;                              // 互斥锁
    int m_wakeup_fd {0};                        // wakeup_fd
    std::shared_ptr<WakeUpFdEvent> m_wakeup_event;  // 用于唤醒epoll_wait的事件
    std::shared_ptr<TimerFdEvent> m_timer;          // 管理所有定时器
};

} // namespace zest

#endif