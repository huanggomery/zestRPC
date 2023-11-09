/* 测试TcpServer以及相应的TCP组件 */
#include "zest/net/tcp/tcp_server.h"
#include "zest/net/rpc/rpc_service.h"
#include "zest/common/logging.h"
#include "protoc/test_service.pb.h"

class TestServiceImpl : public TestService
{
public:
    void sum(google::protobuf::RpcController* controller,
             const ::sumRequest* request,
             ::sumResponse* response,
             ::google::protobuf::Closure* done) override
    {
        int num1 = request->num1();
        int num2 = request->num2();
        response->set_result(num1 + num2);
        if (done) done->Run();
        LOG_DEBUG << "Get RPC request: " << num1 << " + " << num2 << " = " << num1+num2;
    }
};


int main()
{
    zest::Config::SetGlobalConfig("../conf/zest.xml");
    zest::Logger::InitGlobalLogger();
    
    // 注册RPC服务
    zest::RpcService::service_ptr service = std::make_shared<TestServiceImpl>();
    zest::RpcService::GetRpcService()->register_service(service);

    // 构造服务器当前地址
    sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(zest::Config::GetGlobalConfig()->port());
    auto addr_ptr = std::make_shared<zest::IPv4Addr>(local_addr);

    // 构造服务器，并启动
    zest::TcpServer::CreateTcpServer(addr_ptr);
    zest::TcpServer::s_ptr server = zest::TcpServer::GetTcpServer();
    server->start();

    return 0;
}