/* 自定义 RpcController */
#include "zest/net/rpc/rpc_controller.h"

// TODO: RpcController 类什么都没实现！

namespace zest
{

void RpcController::Reset()
{
    m_msg_id.clear();
    m_error_code = RpcProtocol::ERROR_NONE;
    m_error_info.clear();

    m_is_finished = false;
    m_is_cancled = false;
    m_is_finished = false;

    m_timeout = 2000;
}

bool RpcController::Failed() const
{
    return m_is_failed;
}

bool RpcController::Finished() const
{
    return m_is_finished;
}

std::string RpcController::ErrorText() const
{
    return m_error_info;
}

void RpcController::StartCancel()
{
    m_is_cancled = true;
    m_is_failed = true;
    m_is_finished = true;
}

void RpcController::SetFailed(const std::string& reason)
{
    m_error_info = reason;
    m_is_failed = true;
}

void RpcController::SetFinished()
{
    m_is_finished = true;
}

bool RpcController::IsCanceled() const
{
    return m_is_cancled;
}

void RpcController::NotifyOnCancel(google::protobuf::Closure* callback)
{

}

void RpcController::SetError(int32_t error_code, const std::string &info)
{
    m_error_code = error_code;
    m_error_info = info;
    m_is_failed = true;
}

int32_t RpcController::GetErrorCode() const
{
    return m_error_code;
}

void RpcController::SetMsgId(const std::string& msg_id)
{
    m_msg_id = msg_id;
}

std::string RpcController::GetMsgId() const
{
    return m_msg_id;
}

void RpcController::setTimeout(int timeout)
{
    m_timeout = timeout;
}

int RpcController::getTimeout() const
{
    return m_timeout;
}
    
} // namespace zest
