/* 封装IO线程和线程池 */
#ifndef _ZEST_NET_IO_THREAD_H
#define _ZEST_NET_IO_THREAD_H
#include "zest/common/noncopyable.h"
#include "zest/common/sync.h"
#include "zest/net/eventloop.h"
#include <pthread.h>
#include <memory>
#include <vector>

namespace zest
{

class IOThread: public noncopyable
{
public:
    using s_ptr = std::shared_ptr<IOThread>;
public:
    IOThread();
    ~IOThread();
    void start() {m_start_sem.post();}
    void stop() {m_eventloop->stop();}
    EventLoop::s_ptr get_eventloop() {return m_eventloop;}
    pid_t get_tid() const {return m_tid;}
    bool is_valid() const {return m_is_valid;}

private:
    // IO线程函数
    static void* ThreadFunc(void *arg);

private:
    EventLoop::s_ptr m_eventloop {nullptr};   // 每个IO线程拥有一个eventloop
    pthread_t m_thread {0};     // 用于创建和等待IO线程
    pid_t m_tid {0};            // 线程ID
    Sem m_init_sem {0};         // 用于IO线程创建时的同步
    Sem m_start_sem {0};        // 用于启动eventloop
    bool m_is_valid {false};    // 标记是否正在运行
};


class ThreadPool: public noncopyable
{
public:
    using s_ptr = std::shared_ptr<ThreadPool>;

public:
    ThreadPool() = delete;
    explicit ThreadPool(int n);
    ~ThreadPool();
    void start();

    // 按照轮转调度法获取io线程
    IOThread::s_ptr get_io_thread();
    
private:
    int m_thread_num;   // 线程数
    int m_index;        // 下一个要使用的线程
    std::vector<IOThread::s_ptr> m_thread_pool;  // 所有线程的指针
};
    
} // namespace zest

#endif