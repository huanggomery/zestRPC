/* 自定义 RpcController */
#include "zest/net/rpc/rpc_controller.h"

// TODO: RpcController 类什么都没实现！

namespace zest
{

void RpcController::Reset()
{

}

bool RpcController::Failed() const
{
    return true;
}

std::string RpcController::ErrorText() const
{
    return std::string();
}

void RpcController::StartCancel()
{

}

void RpcController::SetFailed(const std::string& reason)
{

}

bool RpcController::IsCanceled() const
{
    return true;
}

void RpcController::NotifyOnCancel(google::protobuf::Closure* callback)
{

}
    
} // namespace zest
