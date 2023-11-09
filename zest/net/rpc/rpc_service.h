/* 封装RPC Service， 负责RPC业务逻辑 */
#ifndef ZEST_NET_RPC_RPC_SERVICE_H
#define ZEST_NET_RPC_RPC_SERVICE_H
#include "zest/net/rpc/rpc_protocol.h"
#include "zest/net/tcp/tcp_connection.h"
#include "zest/common/noncopyable.h"
#include <google/protobuf/service.h>
#include <unordered_map>
#include <memory>
#include <string>


namespace zest
{

// 全局单例模式
class RpcService : public noncopyable
{
public:
    using s_ptr = std::shared_ptr<RpcService>;
    using service_ptr = std::shared_ptr<google::protobuf::Service>;

    // 得到全局唯一的 RpcService 指针
    static s_ptr GetRpcService();

    // RPC处理函数,核心函数！
    void process(RpcProtocol::s_ptr req_protocol, RpcProtocol::s_ptr rsp_protocol, TcpConnection::s_ptr connection);

    // 注册RPC服务
    void register_service(service_ptr service);

private:
    RpcService() = default;

    // 解析 service.method 形式的方法名
    bool parse_method_full_name(const std::string &full_name, std::string &service_name, std::string &method_name);

private:
    std::unordered_map<std::string, service_ptr> m_service_map;

};


// 用于在协议结构体中设置错误信息的函数
void set_protocol_error(RpcProtocol::s_ptr protocol, int32_t error_code, const std::string &error_info);

} // namespace zest


#endif