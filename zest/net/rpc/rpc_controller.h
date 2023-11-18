/* 自定义 RpcController */
#ifndef ZEST_NET_RPC_RPC_CONTROLLER_H
#define ZEST_NET_RPC_RPC_CONTROLLER_H
#include "zest/net/rpc/rpc_protocol.h"
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

    bool Finished() const;

    std::string ErrorText() const override;

    void StartCancel() override;

    void SetFailed(const std::string& reason) override;

    void SetFinished();

    bool IsCanceled() const override;

    void NotifyOnCancel(google::protobuf::Closure* callback) override;

    void SetError(int32_t error_code, const std::string &info);

    int32_t GetErrorCode() const;

    void SetMsgId(const std::string& msg_id);

    std::string GetMsgId() const;

    void setTimeout(int timeout);

    int getTimeout() const;

private:
    std::string m_msg_id;
    int32_t m_error_code {RpcProtocol::ERROR_NONE};
    std::string m_error_info;

    bool m_is_failed {false};
    bool m_is_cancled {false};
    bool m_is_finished {false};

    int m_timeout {5000};  // ms
};
    
} // namespace zest


#endif