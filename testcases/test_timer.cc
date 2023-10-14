/* 测试定时器
 * 一个线程创建一个eventloop线程，
 * 一个线程向eventloop添加周期1s的定时器，并在最后设置 valid = false，因此理论上打印出1～4
 * 一个线程向eventloop添加周期2s定时器，并在最后设置 periodic = false，因此理论上打印出1～5
 * 虽然周期2s的定时器先添加，但随后马上添加了周期1s的定时器，因此set valid先输出
 */
#include "zest/net/timer.h"
#include "zest/net/eventloop.h"
#include "zest/common/logging.h"
#include <pthread.h>
#include <unistd.h>
#include <memory>
#include <thread>
#include <chrono>

using SP_EventLoop = std::shared_ptr<zest::EventLoop>;
using SP_TimerEvent = std::shared_ptr<zest::TimerEvent>;
SP_EventLoop g_event_loop = nullptr;

void *eventloop_thread(void *)
{
    g_event_loop = zest::EventLoop::CreateEventLoop();
    g_event_loop->loop();
    return nullptr;
}

// 测试周期性定时器，以及set_valid
void *set_valid_test(void *)
{
    volatile int i = 0;
    SP_TimerEvent timer_event(new zest::TimerEvent(
        1000, 
        [&i](){LOG_DEBUG << "set valid test: i = " << i; ++i;},
        true)
    );
    while (!g_event_loop) {}
    g_event_loop->addTimerEvent(timer_event);
    while (i != 5) {}
    timer_event->set_valid(false);   // 应该只会打印出1～4,不会打印5
    
    sleep(2);
    return nullptr;
}

// 测试周期性定时器，以及set_periodic
void *set_periodic_test(void *)
{
    volatile int i = 0;
    SP_TimerEvent timer_event(new zest::TimerEvent(
        2000, 
        [&i](){LOG_DEBUG << "set periodic test: i = " << i; ++i;},
        true)
    );
    while (!g_event_loop) {}
    g_event_loop->addTimerEvent(timer_event);
    while (i != 5) {}
    timer_event->set_periodic(false);   // 会打印出1～5
    
    sleep(3);
    return nullptr;
}

int main()
{
    zest::Config::SetGlobalConfig("../conf/zest.xml");
    zest::Logger::InitGlobalLogger();
    
    pthread_t event_loop_tid, timer_tid1, timer_tid2;
    pthread_create(&event_loop_tid, NULL, eventloop_thread, NULL);
    pthread_create(&timer_tid1, NULL, set_periodic_test, NULL);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    pthread_create(&timer_tid2, NULL, set_valid_test, NULL);


    pthread_join(timer_tid1, NULL);
    pthread_join(timer_tid2, NULL);
    sleep(4);
    g_event_loop->stop();
    pthread_join(event_loop_tid, NULL);
    return 0;
}