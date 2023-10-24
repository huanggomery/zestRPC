/* 对网络地址的封装 */
#ifndef ZEST_NET_TCP_NET_ADDR_H
#define ZEST_NET_TCP_NET_ADDR_H
#include <memory>
#include <arpa/inet.h>
#include <string>

namespace zest
{

// 所有地址类型的虚基类
class NetAddrBase
{
public:
    using s_ptr = std::shared_ptr<NetAddrBase>;

public:
    virtual sockaddr* get_sock_addr() = 0;
    virtual socklen_t get_sock_len() const = 0;
    virtual int get_family() const = 0;
    virtual std::string to_string() const = 0;
    virtual bool check() const = 0; 
};


// IPv4地址类型
class IPv4Addr : public NetAddrBase
{
public:
    IPv4Addr() = delete;
    ~IPv4Addr() = default;

    // 各种形式的构造函数
    IPv4Addr(const std::string &addr_str);
    IPv4Addr(const std::string &ip_str, uint16_t port);
    IPv4Addr(const char *addr_str);
    IPv4Addr(const char *ip_str, uint16_t port);
    IPv4Addr(sockaddr_in addr);
    IPv4Addr(in_addr_t ip, uint16_t port);

    sockaddr* get_sock_addr() override;
    socklen_t get_sock_len() const override;
    int get_family() const override;
    std::string to_string() const override;
    bool check() const override;
    
private:
    std::string m_ip {"0.0.0.0"};     // 点分十进制表示的IP地址
    uint16_t m_port {0};
    sockaddr_in m_sockaddr;
};

// 其它地址类型......

    
} // namespace zest


#endif