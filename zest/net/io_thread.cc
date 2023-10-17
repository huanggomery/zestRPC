/* 封装IO线程和线程池 */
#include "zest/net/io_thread.h"
#include "zest/common/logging.h"
#include "zest/common/util.h"
#include <stdexcept>
#include <iostream>

namespace zest
{

IOThread::IOThread()
{
    int rt = pthread_create(&m_thread, NULL, IOThread::ThreadFunc, this);
    if (rt < 0) {
        LOG_ERROR << "Create IO thread failed";
        throw std::runtime_error("Create IO thread failed");
    }
    // 等待IO线程完成一些初始化操作
    m_init_sem.wait();
}

IOThread::~IOThread()
{
    stop();
    pthread_join(m_thread, NULL);
}

// IO线程函数
void* IOThread::ThreadFunc(void *arg)
{
    IOThread *thread = static_cast<IOThread*>(arg);
    thread->m_eventloop = EventLoop::CreateEventLoop();
    thread->m_tid = getTid();
    if (!thread->m_eventloop) {
        LOG_ERROR << "IO thread get null eventloop ptr";
        thread->m_init_sem.post();
        return nullptr;
    }
    thread->m_is_valid = true;

    // IO线程初始化工作完成，唤醒主线程的构造函数
    thread->m_init_sem.post();

    // 等待主线程中调用 start() 启动eventloop循环
    thread->m_start_sem.wait();

    thread->m_eventloop->loop();
    thread->m_is_valid = false;

    LOG_INFO << "IO thread exit normally";
    return nullptr;
}

ThreadPool::ThreadPool(int n): m_thread_num(n), m_index(0), m_thread_pool(n, nullptr)
{
    for (int i = 0; i < n; ++i) {
        try {
            m_thread_pool[i].reset(new IOThread());
        }
        catch(const std::runtime_error& e) {
            std::cerr << e.what() << '\n';
            break;
        }
        catch(const std::bad_alloc& e) {
            std::cerr << e.what() << '\n';
            break;
        }
    }
}

ThreadPool::~ThreadPool()
{
    LOG_INFO << "Thread pool exit";
}

void ThreadPool::start()
{
    LOG_INFO << "Thread pool start";
    for (int i = 0; i < m_thread_num; ++i) {
        if (m_thread_pool[i])
            m_thread_pool[i]->start();
    }
}

// 按照轮转调度法获取io线程
IOThread::s_ptr ThreadPool::get_io_thread()
{
    // 考虑到IO线程中 EventLoop::CreateEventLoop() 可能失败，所以要判断eventloop是否在运行
    if (m_thread_pool[m_index] && m_thread_pool[m_index]->is_valid()) {
        auto rt = m_thread_pool[m_index];
        m_index = (m_index + 1) % m_thread_num;
        return rt;
    }
    
    for (int i = (m_index+1) % m_thread_num; i != m_index; i = (i+1) % m_thread_num) {
        if (m_thread_pool[i] && m_thread_pool[i]->is_valid()) {
            m_index = (i + 1) % m_thread_num;
            return m_thread_pool[i];
        }
    }

    // 不太可能执行到这里，除非每个线程的 EventLoop::CreateEventLoop() 都失败了
    LOG_FATAL << "NO IO thread can be used";
    exit(-1);
}

} // namespace zest
