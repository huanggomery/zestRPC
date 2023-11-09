/* 封装RPC Service， 负责RPC业务逻辑 */
#include "zest/net/rpc/rpc_service.h"
#include "zest/net/rpc/rpc_controller.h"
#include "zest/net/rpc/rpc_closure.h"
#include "zest/common/logging.h"
#include <google/protobuf/message.h>
#include <stdexcept>


namespace zest
{

using msg_ptr = std::shared_ptr<google::protobuf::Message>;

static RpcService::s_ptr g_rpc_service = nullptr;

// 返回全局唯一的 RpcService 指针，如果没有则先构造后返回
RpcService::s_ptr RpcService::GetRpcService()
{
    if (g_rpc_service) return g_rpc_service;

    try
    {
        g_rpc_service.reset(new RpcService());
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        LOG_FATAL << "Create RpcService failed";
        exit(-1);
    }
    
    return g_rpc_service;
}

// RPC处理函数，核心函数！
void RpcService::process(RpcProtocol::s_ptr req_protocol, RpcProtocol::s_ptr rsp_protocol, TcpConnection::s_ptr connection)
{
    // 首先解析服务名和方法名
    std::string method_full_name = req_protocol->m_method_name;
    std::string service_name, method_name;
    if (parse_method_full_name(method_full_name, service_name, method_name) == false) {
        set_protocol_error(rsp_protocol, RpcProtocol::ERROR_PARSE_METHOD_NAME, "parse method name error");
        return;
    }

    // 根据服务名，从注册的服务中获取服务
    auto it = m_service_map.find(service_name);
    if (it == m_service_map.end()) {
        LOG_ERROR << "service name [" << service_name << "] not found";
        set_protocol_error(rsp_protocol, RpcProtocol::ERROR_SERVICE_NOT_FOUND, "service not found");
        return;
    }
    service_ptr service = it->second;

    // 根据方法名，从服务中获取方法
    auto method_desc = service->GetDescriptor()->FindMethodByName(method_name);
    if (method_desc == nullptr) {
        LOG_ERROR << "method name [" << method_name << "] is not found in service [" << service_name << "]";
        set_protocol_error(rsp_protocol, RpcProtocol::ERROR_SERVICE_NOT_FOUND, "method not found");
        return;
    }

    // 构造出方法的请求对象，即函数的输入参数
    msg_ptr req_msg(service->GetRequestPrototype(method_desc).New());

    // 把 req_protocol 的 m_rpc_data 反序列化为 req_msg
    if (req_msg->ParseFromString(req_protocol->m_rpc_data) == false) {
        LOG_ERROR << "[" << method_full_name << "] deserialize error";
        set_protocol_error(rsp_protocol, RpcProtocol::ERROR_DESERIALIZE_FAILED, "deserialize error");
        return;
    }

    // 构造出方法的响应对象，即函数的输出
    msg_ptr rsp_msg(service->GetResponsePrototype(method_desc).New());

    // 定义一个 RpcController
    std::shared_ptr<RpcController> rpc_controller(new RpcController());

    // 定义一个 RpcClosure，用于完成RPC调用后回调
    // 作用是填充 rsp_protocol 的各个字段，然后encoder，最后放入 TcpConnection 的发送缓存
    std::shared_ptr<RpcClosure> rpc_closure(new RpcClosure(
        [method_full_name, rsp_protocol, rsp_msg, connection](){
            // 将方法的响应对象序列化，并填充到响应协议中
            if (rsp_msg->SerializeToString(&(rsp_protocol->m_rpc_data)) == false) {
                LOG_ERROR << "[" << method_full_name << "] serialize error";
                rsp_protocol->m_rpc_data.clear();
                set_protocol_error(rsp_protocol, RpcProtocol::ERROR_SERIALIZE_FAILED, "serialize error");
            }
            else {
                set_protocol_error(rsp_protocol, RpcProtocol::ERROR_NONE, "");
            }

            // 计算响应协议的整包长度
            rsp_protocol->m_pk_len = 
                1 + 4 + 4 + rsp_protocol->m_msg_id_len + 
                4 + rsp_protocol->m_method_name_len + 
                4 + 4 + rsp_protocol->m_error_info_len + 
                rsp_protocol->m_rpc_data.size() + 4 + 1;

            // 计算并填充校验和
            fill_check_sum(rsp_protocol);

            // 将协议编码成字节序列后放入发送缓存
            RpcEncoder(rsp_protocol, connection->out_buffer());
        }
    ));
    
    /* ------------------ 真正的RPC调用 ---------------------- */
    service->CallMethod(method_desc, rpc_controller.get(), req_msg.get(), rsp_msg.get(), rpc_closure.get());
    /* ----------------------------------------------------- */
}

// 注册RPC服务
void RpcService::register_service(service_ptr service)
{
    std::string service_name = service->GetDescriptor()->full_name();

    // 判断是否已经注册同名的服务
    if (m_service_map.find(service_name) != m_service_map.end()) {
        LOG_ERROR << "register the same service";
        throw std::runtime_error("register the same service");
    }
    m_service_map[service_name] = service;
}

// 解析 service.method 形式的方法名
bool RpcService::parse_method_full_name(const std::string &full_name, std::string &service_name, std::string &method_name)
{
    std::size_t pos = full_name.find(".");
    if (pos == std::string::npos) {
        LOG_ERROR << "The format of method name is wrong, miss '.'";
        return false;
    }

    service_name = full_name.substr(0, pos);
    method_name = full_name.substr(pos+1);
    return true;
}

// 用于在协议结构体中设置错误信息的函数
void set_protocol_error(RpcProtocol::s_ptr protocol, int32_t error_code, const std::string &error_info)
{
    protocol->m_error_code = error_code;
    protocol->m_error_info = error_info;
    protocol->m_error_info_len = error_info.size();
}

} // namespace zest
