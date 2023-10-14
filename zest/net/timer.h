/* 定时器 */
#ifndef _ZEST_NET_TIMER_H
#define _ZEST_NET_TIMER_H
#include "zest/net/fd_event.h"
#include "zest/common/sync.h"
#include "zest/common/util.h"
#include <memory>
#include <queue>
#include <vector>

namespace zest
{

// 单个定时器任务，记录超时时间和回调函数
class TimerEvent
{
public:
    using s_ptr = std::shared_ptr<TimerEvent>;
    using CallBackFunc = std::function<void()>;  // 回调函数

    TimerEvent() = delete;
    TimerEvent(int64_t interval, CallBackFunc cb, bool periodic = false);
    ~TimerEvent() = default;

    int64_t getInterval() const {return m_interval;}
    int64_t getTriggerTime() const {return m_trigger_time;}
    CallBackFunc handler() const {return m_callback;}
    bool is_periodic() const {return m_periodicity;}
    bool is_valid() const {return m_valid;}
    void set_periodic(bool value) {m_periodicity = value;}
    void set_valid(bool value) {m_valid = value;}
    void reset_time() {m_trigger_time = get_now_ms() + m_interval;}

private:
    int64_t m_interval;        // 时间间隔，单位 ms
    int64_t m_trigger_time;    // 触发时间，单位 ms
    CallBackFunc m_callback;   // 定时器回调函数
    bool m_periodicity;        // 周期性事件
    bool m_valid;              // 是否有效
};


/* 用一个timerfd和一个队列管理多个定时器
 * TimerFd所维护的timerfd实际上是优先队列中第一个（就是最早一个）定时器的触发时间 
 * 每当timerfd触发，就依次检查优先队列中的定时器，执行所有到期的定时器的回调函数
 */
class TimerFdEvent: public FdEvent
{
    using SP_Timer = std::shared_ptr<TimerEvent>;
    struct CmpTimer  // 用于在优先队列中对定时器进行排序，
    {
        bool operator()(SP_Timer t1, SP_Timer t2)
        {
            return t1->getTriggerTime() > t2->getTriggerTime();
        }
    };
    using TimerQueue = std::priority_queue<SP_Timer, std::vector<SP_Timer>, CmpTimer>;
    
public:
    TimerFdEvent();
    ~TimerFdEvent() = default;
    void addTimerEvent(SP_Timer timer);
    void deleteTimerEvent(SP_Timer timer);

public:
    void handleTimerEvent();  // 处理定时任务函数
    void resetTimerFd();      // 重新设置定时器fd
    
private:
    TimerQueue m_pending_timers;   // 用优先队列管理所有定时器事件
    Mutex m_mutex;
};

} // namespace zest

#endif