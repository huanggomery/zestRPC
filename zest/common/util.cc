/* 定义一些工具函数 */
#include "zest/common/util.h"
#include "zest/common/logging.h"
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <stdexcept>

namespace zest
{

static pid_t g_pid = 0;
static thread_local pid_t t_tid = 0;

// 生成日志文件名
std::string get_logfile_name(const std::string &file_name, const std::string &file_path)
{
    char str_t[25] = {0};
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t sec = tv.tv_sec;
    struct tm *ptime = localtime(&sec);
    strftime(str_t, 26, "%Y%m%d%H%M%S", ptime);

    return std::string(file_path + file_name + str_t + ".log");
}


// 返回当前时间的字符串,格式类似于： 20230706 21:05:57.229383
std::string get_time_str()
{
    char str_t[27] = {0};
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t sec = tv.tv_sec;
    struct tm *ptime = localtime(&sec);
    strftime(str_t, 26, "%Y-%m-%d %H:%M:%S", ptime);

    char usec[8];
    snprintf(usec, sizeof(usec), ".%ld", tv.tv_usec);
    strcat(str_t, usec);

    return std::string(str_t);
}

// 返回当前时间的ms表示
int64_t get_now_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// 返回进程ID
pid_t getPid()
{
    return g_pid != 0 ? g_pid : (g_pid = getpid());
}

// 返回线程ID
pid_t getTid()
{
    return t_tid != 0 ? t_tid : (t_tid = syscall(SYS_gettid));
}

// 检测文件夹是否存在
bool folderExists(const std::string& folderPath)
{
    struct stat info;

    if (stat(folderPath.c_str(), &info) != 0) {
        return false; // stat 函数失败，文件夹不存在
    }

    return S_ISDIR(info.st_mode);
}

// 将文件描述符设置为非阻塞
void set_non_blocking(int fd)
{
    int old_opt = fcntl(fd, F_GETFL);
    if (old_opt & O_NONBLOCK)
        return;
    fcntl(fd, F_SETFL, old_opt | O_NONBLOCK);
}

// 创建服务器套接字,如果失败，返回-1
int create_socket_and_listen(int port)
{
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        // std::cerr << "create socket failed" << std::endl;
        LOG_ERROR <<"Create socket failed";
        return -1;
    }

    // 终止time-wait
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // 禁用Nagle算法
    int flag = 1;
    setsockopt(listenfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

    if (bind(listenfd, (sockaddr *)&address, sizeof(address)) == -1) {
        LOG_ERROR << "bind failed";
        close(listenfd);
        return -1;
    }

    if (listen(listenfd, 2048) == -1) {
        LOG_ERROR << "listen failed";
        close(listenfd);
        return -1;
    }

    set_non_blocking(listenfd);
    return listenfd;
}

// 创建客户端套接字并连接服务器，如果失败，则返回-1
int create_client_and_connect(const char *ip, int port)
{
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(port);

    int clnfd = socket(PF_INET, SOCK_STREAM, 0);
    if (clnfd == -1) {
        // std::cerr << "create socket failed" << std::endl;
        LOG_ERROR <<"Create socket failed";
        return -1;
    }
    // 禁用Nagle算法
    int flag = 1;
    setsockopt(clnfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

    if (connect(clnfd, (sockaddr*)&address, sizeof(address)) == -1) {
        LOG_ERROR << "connect failed";
        close(clnfd);
        return -1;
    }
    return clnfd;
}

// 根据sockaddr_in，返回IP地址的点分十进制字符串
std::string dotted_decimal_notation(const sockaddr_in &addr)
{
    return std::string(inet_ntoa(addr.sin_addr));
}

// 根据sockaddr_in，返回源端口号
int src_port(const sockaddr_in &addr)
{
    return ntohs(addr.sin_port);
}

// 从网络字节序列中读取一个int32
int32_t get_int32_from_net_bytes(const char *buf)
{
    int32_t result;
    memcpy(&result, buf, 4);
    return ntohl(result);
}

// 返回本地IPv4地址
sockaddr_in get_local_addr(int fd)
{
    sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    socklen_t len = sizeof(local_addr);

    if (getsockname(fd, (sockaddr*)&local_addr, &len) == -1) {
        LOG_ERROR << "getsockname failed";
        throw std::runtime_error("getsockname failed");
    }

    return local_addr;
}

} // namespace zest
