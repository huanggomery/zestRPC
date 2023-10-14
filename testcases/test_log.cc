/* 测试日志系统
 * 创建多个线程，并发地向日志系统写入从0～max_count的整数
 * 可用analysis_log程序分析写入的正确性
 */
#include <iostream>
#include "zest/common/config.h"
#include "zest/common/logging.h"
#include <unistd.h>
#include <pthread.h>
#include <thread>
#include <chrono>
using namespace std;

const int N = 8;                 // 线程数
const int max_count = 100000;    // 每个线程写入条数

void *threadFunc(void *arg)
{
    int ms = 0;
    int *pi = static_cast<int *>(arg);
    int num = *pi;
    for (int i = 0; i < max_count; ++i) {
        LOG_DEBUG << "thread: " << num << " " << i;
        if (i % 1000 == 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    return NULL;
}

int main()
{
    zest::Config::SetGlobalConfig("../conf/zest.xml");
    zest::Logger::InitGlobalLogger();
    pthread_t tid[N];
    int thread_nums[N];

    for (int i = 0; i < N; ++i) {
        thread_nums[i] = i+1;
        pthread_create(tid+i, NULL, threadFunc, thread_nums+i);
    }

    for (int i = 0; i < N; ++i)
        pthread_join(tid[i], NULL);

    return 0;
}