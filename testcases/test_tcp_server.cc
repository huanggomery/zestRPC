/* 测试TcpServer以及相应的TCP组件 */
#include "zest/net/tcp/tcp_server.h"
#include "zest/common/logging.h"



int main()
{
    zest::Config::SetGlobalConfig("../conf/zest.xml");
    zest::Logger::InitGlobalLogger();
    
    sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(12345);

    auto addr_ptr = std::make_shared<zest::IPv4Addr>(local_addr);
    zest::TcpServer server(addr_ptr);
    server.start();

    return 0;
}