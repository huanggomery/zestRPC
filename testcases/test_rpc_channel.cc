#include "zest/net/rpc/rpc_channel.h"
#include "zest/net/rpc/rpc_closure.h"
#include "zest/net/rpc/rpc_controller.h"
#include "zest/net/tcp/net_addr.h"
#include "zest/common/logging.h"
#include "zest/common/config.h"
#include "protoc/test_service.pb.h"
#include <google/protobuf/service.h>
#include <iostream>


int main()
{
    zest::Config::SetGlobalConfig("../conf/zest.xml");
    zest::Logger::InitGlobalLogger();

    zest::NetAddrBase::s_ptr peer_addr = std::make_shared<zest::IPv4Addr>("127.0.0.1", 12345);
    google::protobuf::RpcChannel *channel = new zest::RpcChannel(peer_addr);
    TestService_Stub *service = new TestService_Stub(channel);
    zest::RpcClosure *closure = new zest::RpcClosure();
    zest::RpcController *controller = new zest::RpcController();

    sumRequest *req = new sumRequest();
    sumResponse *rsp = new sumResponse();
    int a, b;
    std::cout << "Input 2 number: " << std::endl;
    std::cin >> a >> b;
    req->set_num1(a);
    req->set_num2(b);
    service->sum(controller, req, rsp, closure);
    std::cout << rsp->result() << std::endl;

    return 0;
}