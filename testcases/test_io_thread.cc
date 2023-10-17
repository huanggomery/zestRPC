/* 测试IO线程池 
 * 线程池中每个线程都负责监听定时器
 */
#include "zest/net/io_thread.h"
#include "zest/net/fd_event.h"
#include "zest/net/timer.h"
#include "zest/common/logging.h"
#include <unistd.h>

using SP_TimerEvent = std::shared_ptr<zest::TimerEvent>;

int main()
{
    zest::Config::SetGlobalConfig("../conf/zest.xml");
    zest::Logger::InitGlobalLogger();

    // 创建一个有3个线程的线程池
    zest::ThreadPool::s_ptr thread_pool(new zest::ThreadPool(3));

    int i = 1, j = 1, k = 1;
    SP_TimerEvent timer_event1(new zest::TimerEvent(
        1000, 
        [&i](){LOG_DEBUG << "set valid test: i = " << i++; },
        true)
    );
    SP_TimerEvent timer_event2(new zest::TimerEvent(
        2000, 
        [&j](){LOG_DEBUG << "set valid test: j = " << j++; },
        true)
    );
    SP_TimerEvent timer_event3(new zest::TimerEvent(
        3000, 
        [&k](){LOG_DEBUG << "set valid test: k = " << k++; },
        true)
    );

    thread_pool->get_io_thread()->get_eventloop()->addTimerEvent(timer_event1);
    thread_pool->get_io_thread()->get_eventloop()->addTimerEvent(timer_event2);
    thread_pool->get_io_thread()->get_eventloop()->addTimerEvent(timer_event3);
    thread_pool->start();
    
    sleep(10);
    return 0;
}