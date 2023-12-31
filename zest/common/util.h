/* 定义一些工具函数 */
#ifndef ZEST_COMMON_UTIL_H
#define ZEST_COMMON_UTIL_H
#include <string>
#include <arpa/inet.h>

namespace zest
{

// 生成日志文件名
std::string get_logfile_name(const std::string &file_name, const std::string &file_path);

// 返回当前时间的字符串,格式类似于： 20230706 21:05:57.229383
std::string get_time_str();

// 返回当前时间的ms表示
int64_t get_now_ms();

// 返回进程ID
pid_t getPid();

// 返回线程ID
pid_t getTid();

// 检测文件夹是否存在
bool folderExists(const std::string& folderPath);

// 将文件描述符设置为非阻塞
void set_non_blocking(int fd);

// 创建服务器套接字,如果失败，返回-1
int create_socket_and_listen(int port);

// 创建客户端套接字并连接服务器，如果失败，则返回-1
int create_client_and_connect(const char *ip, int port);

// 根据sockaddr_in，返回IP地址的点分十进制字符串
std::string dotted_decimal_notation(const sockaddr_in &addr);

// 根据sockaddr_in，返回源端口号
int src_port(const sockaddr_in &addr);

// 从网络字节序列中读取一个int32
int32_t get_int32_from_net_bytes(const char *buf);

// 返回本地IPv4地址
sockaddr_in get_local_addr(int fd);

} // namespace zest


#endif