/* 自定义RpcChannel */
#ifndef ZEST_NET_RPC_RPC_CHANNEL_H
#define ZEST_NET_RPC_RPC_CHANNEL_H
#include "zest/net/tcp/net_addr.h"
#include "zest/common/noncopyable.h"
#include <google/protobuf/service.h>
#include <memory>
#include <string>


namespace zest
{

class RpcChannel: public google::protobuf::RpcChannel, public noncopyable
{
public:
    RpcChannel() = delete;   // 构造的时候必须指定服务端地址

    RpcChannel(NetAddrBase::s_ptr peer_addr);

    ~RpcChannel() = default;

    void CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                    google::protobuf::Message* response, google::protobuf::Closure* done) override;
private:
    NetAddrBase::s_ptr m_peer_addr;

};
    
} // namespace zest



#endif