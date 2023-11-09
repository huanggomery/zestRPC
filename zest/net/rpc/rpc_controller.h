/* 自定义 RpcController */
#ifndef ZEST_NET_RPC_RPC_CONTROLLER_H
#define ZEST_NET_RPC_RPC_CONTROLLER_H
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <string>


namespace zest
{

class RpcController : public google::protobuf::RpcController
{
public:
    RpcController() = default;
    
    ~RpcController() override { /* do nothing */};

    void Reset() override;

    bool Failed() const override;

    std::string ErrorText() const override;

    void StartCancel() override;

    void SetFailed(const std::string& reason) override;

    bool IsCanceled() const override;

    void NotifyOnCancel(google::protobuf::Closure* callback) override;

};
    
} // namespace zest


#endif