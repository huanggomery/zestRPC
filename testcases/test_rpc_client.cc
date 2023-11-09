/* 测试RPC服务的客户端 */
#include "zest/common/config.h"
#include "zest/common/util.h"
#include "zest/common/logging.h"
#include "zest/net/rpc/rpc_protocol.h"
#include "zest/net/rpc/rpc_service.h"
#include "zest/net/tcp/tcp_buffer.h"
#include "protoc/test_service.pb.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <unistd.h>

void fill_protocol(zest::RpcProtocol::s_ptr protocol);

int32_t MsgId = 516;

int main()
{
    zest::Config::SetGlobalConfig("../conf/zest.xml");

    int clientfd = zest::create_client_and_connect("127.0.0.1", zest::Config::GetGlobalConfig()->port());
    if (clientfd == -1) {
        std::cerr << "create_client_and_connect failed" << std::endl;
        exit(-1);
    }
    zest::set_non_blocking(clientfd);

    // 发送RPC请求的协议
    zest::RpcProtocol::s_ptr req_protocol = std::make_shared<zest::RpcProtocol>();
    req_protocol->clear();

    // 发送和接收缓存
    zest::TcpBuffer in_buffer;
    zest::TcpBuffer out_buffer;

    std::string input;
    std::cout << "Please input more than 2 numbers: " << std::endl;
    while (std::getline(std::cin, input)) {
        std::istringstream iss(input);
        std::vector<int> vec;
        int num;
        while (iss >> num)
            vec.push_back(num);

        // 将cin输入的数字都封装成RPC协议，序列化后放入发送缓存
        std::vector<int> req_nums;
        for (int i : vec) {
            req_nums.push_back(i);
            if (req_nums.size() == 2) {
                sumRequest request;
                request.set_num1(req_nums[0]);
                request.set_num2(req_nums[1]);
                req_nums.clear();

                request.SerializeToString(&req_protocol->m_rpc_data);
                fill_protocol(req_protocol);

                zest::RpcEncoder(req_protocol, out_buffer);
            }
        }

        // 发送RPC请求数据
        int send_len = 0;
        while (!out_buffer.empty()) {
            const char *ptr = out_buffer.c_str();
            int len = send(clientfd, ptr, out_buffer.size(), 0);
            if (len == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                else if (errno == EINTR)
                    continue;
                else {
                    close(clientfd);
                    std::cerr << "send error" << std::endl;
                    exit(-1);
                }
            }
            out_buffer.move_forward(len);
            send_len += len;
        }
        std::cout << "send " << send_len << " bytes data to server" << std::endl;

        // 等待1s，然后接收RPC响应
        sleep(1);
        char tmp_buf[1024];
        while (1) {
            memset(tmp_buf, 0, sizeof(tmp_buf));
            int len = recv(clientfd, tmp_buf, sizeof(tmp_buf), 0);
            if (len < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                else if (errno == EINTR)
                    continue;
                else {
                    close(clientfd);
                    std::cerr << "recv error" << std::endl;
                    exit(-1);
                }
            }
            else if (len == 0) {
                close(clientfd);
                std::cerr << "disconnected by server" << std::endl;
                exit(-1);
            }
            // 正常情况
            else {
                in_buffer.append(tmp_buf, len);
            }
        }

        zest::RpcDecoder decoder(in_buffer);
        zest::RpcProtocol::s_ptr rsp_protocol;
        std::cout << "result: ";
        while (rsp_protocol = decoder.decode()) {
            sumResponse response;
            response.ParseFromString(rsp_protocol->m_rpc_data);
            std::cout << response.result() << " ";
        }
        std::cout << std::endl;
        std::cout << "Please input more than 2 numbers: " << std::endl;
    }
}


void fill_protocol(zest::RpcProtocol::s_ptr protocol)
{
    protocol->m_msg_id = std::to_string(MsgId++);
    protocol->m_msg_id_len = protocol->m_msg_id.size();
    protocol->m_method_name = "TestService.sum";
    protocol->m_method_name_len = protocol->m_method_name.size();
    zest::set_protocol_error(protocol, zest::RpcProtocol::ERROR_NONE, "");
    zest::fill_check_sum(protocol);

    protocol->m_pk_len = 
        1 + 4 + 4 + protocol->m_msg_id_len + 
        4 + protocol->m_method_name_len + 
        4 + 4 + protocol->m_error_info_len + 
        protocol->m_rpc_data.size() + 4 + 1;
}