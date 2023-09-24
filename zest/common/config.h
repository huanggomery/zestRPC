#ifndef ZEST_COMMON_CONFIG_H
#define ZEST_COMMON_CONFIG_H

#include <string>

namespace zest
{
    
class Config
{
public:
    static Config *GetGlobalConfig();   // 返回Config指针
    static void SetGlobalConfig(const char *xmlfile);  // 工厂函数
    static void ShowGlobalConfig();    // 打印配置参数

    Config();
    Config(const char *xmlfile);
    ~Config();

public:
    std::string log_level() const {return m_log_level;}
    std::string log_file_name() const {return m_log_file_name;}
    std::string log_file_path() const {return m_log_file_path;}
    int log_max_file_size() const {return m_log_max_file_size;}
    int log_sync_interval() const {return m_log_sync_interval;}
    int log_max_buffers() const {return m_log_max_buffers;}
    int port() const {return m_server_port;}
    int io_threads() const {return m_server_io_threads;}
    
private:
    std::string m_log_level;
    std::string m_log_file_name;
    std::string m_log_file_path;
    int m_log_max_file_size;    // 单个日志文件中最大记录条数
    int m_log_sync_interval;    // 日志刷盘间隔，单位ms
    int m_log_max_buffers;      // 日志系统拥有的最多缓冲区数目

    int m_server_port;       // 端口号
    int m_server_io_threads; // 线程数

private:
    static Config *g_config;  // 单例模式指针
};

} // namespace zest

#endif