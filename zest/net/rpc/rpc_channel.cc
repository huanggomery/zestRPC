/* 自定义RpcChannel */
#include "zest/net/rpc/rpc_channel.h"
#include "zest/net/rpc/rpc_controller.h"
#include "zest/net/rpc/rpc_closure.h"
#include "zest/net/tcp/tcp_client.h"
#include "zest/net/rpc/rpc_protocol.h"


namespace zest
{

RpcChannel::RpcChannel(NetAddrBase::s_ptr peer_addr): m_peer_addr(peer_addr)
{
    /* do nothing */
}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                    google::protobuf::Message* response, google::protobuf::Closure* done)
{
    RpcController *my_controller = dynamic_cast<RpcController*>(controller);
    RpcClosure *my_closure = dynamic_cast<RpcClosure*>(done);

    // 创建 TcpClient 对象，并连接服务器
    TcpClient::s_ptr client = std::make_shared<TcpClient>(m_peer_addr);
    client->connect();
    if (client->get_state() == zest::NotConnected) {
        my_controller->SetError(RpcProtocol::ERROR_CONNECT_FAILED, "connect failed");
        return;
    }

    // 填充 req_protocol 的各个字段，然后序列化后放入发送缓存，等待发送
    std::string method_fullname = method->full_name();
    client->prepare(request, method_fullname);

    // 设置超时定时器
    TimerEvent::s_ptr timer_event = std::make_shared<TimerEvent>(
        my_controller->getTimeout(), 
        [my_controller, client]() {
            if (my_controller->Finished()) {
                client->stop();
                return;
            }
            my_controller->StartCancel();
            my_controller->SetError(RpcProtocol::ERROR_CALL_TIMEOUT, "rpc call timeout");
            client->stop();
        });
    client->addTimerEvent(timer_event);

    client->start();  // 处理完业务逻辑后会退出eventloop循环

    RpcProtocol::s_ptr rsp_protocol = client->get_rsp_protocol();

    // 如果返回的协议为空，则可能是服务端关闭连接，或者字节序列解析出错
    if (rsp_protocol == nullptr) {
        my_controller->SetError(RpcProtocol::ERROR_CALL_FAILED, "rpc call failed");
    }
    else {
        response->ParseFromString(rsp_protocol->m_rpc_data);
    }

    my_controller->SetFinished();
    if (done) done->Run();
}
    
} // namespace zest
