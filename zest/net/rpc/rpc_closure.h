/* 自定义 RpcClosure */
#ifndef ZEST_NET_RPC_RPC_CLOSURE_H
#define ZEST_NET_RPC_RPC_CLOSURE_H
#include <google/protobuf/service.h>
#include <functional>


namespace zest
{

class RpcClosure : public google::protobuf::Closure
{
public:
    RpcClosure() = default;
    RpcClosure(std::function<void()> cb) : m_cb(cb) { /* do nothing */}

    void Run() override
    {
        if (m_cb) m_cb();
    }

private:
    std::function<void()> m_cb {nullptr};
};
    
} // namespace zest




#endif