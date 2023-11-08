/* RPC协议的封装，用有限状态机作为解码器解析协议，以及编码器函数 */
#ifndef ZEST_NET_RPC_RPC_PROTOCOL_H
#define ZEST_NET_RPC_RPC_PROTOCOL_H
#include "zest/net/tcp/tcp_buffer.h"
#include "zest/common/noncopyable.h"
#include <sys/types.h>
#include <string>
#include <memory>


namespace zest
{

struct RpcProtocol
{
    using s_ptr = std::shared_ptr<RpcProtocol>;

    static const char START_FLAG = 0x02;  // 开始符
    static const char END_FLAG = 0x03;    // 结束符
    
    int32_t m_pk_len {0};           // 整包长度，包括开始符和结束符
    int32_t m_msg_id_len {0};       // MsgID长度
    std::string m_msg_id;           // MsgID
    int32_t m_method_name_len {0};  // 方法名长度
    std::string m_method_name;      // 方法名
    int32_t m_error_code {0};       // 错误码
    int32_t m_error_info_len {0};   // 错误信息长度
    std::string m_error_info;       // 错误信息
    std::string m_rpc_data;         // 序列化的RPC数据
    int32_t m_check_sum {0};        // 校验和
};


// 用有限状态机解析协议
class RpcDecoder : public noncopyable
{
    enum MainState {
        STATE_WAITING_START_FLAG = 0,
        STATE_PARSE_PK_LEN,
        STATE_PARSE_MSG_ID_LEN,
        STATE_PARSE_MSG_ID,
        STATE_PARSE_METHOD_NAME_LEN,
        STATE_PARSE_METHOD_NAME,
        STATE_PARSE_ERROR_CODE,
        STATE_PARSE_ERROR_INFO_LEN,
        STATE_PARSE_ERROR_INFO,
        STATE_PARSE_RPC_DATA,
        STATE_PARSE_CHECK_SUM,
        STATE_WAITING_END_FLAG,
        STATE_VALID_CHECK_SUM,
        STATE_PARSE_SUCCESS,
        STATE_PARSE_FAILED,
    };

public:
    RpcDecoder() = delete;
    RpcDecoder(TcpBuffer &in_buffer);
    ~RpcDecoder() = default;

    // 解析 m_in_buffer 序列，返回 RpcProtocol 指针
    RpcProtocol::s_ptr decode();

    // 判断是否出错
    bool is_failed() const {return m_state == STATE_PARSE_FAILED;}

private:
    void clear();
    bool valid_check_sum() const;
    
private:
    RpcProtocol::s_ptr m_req_protocol;
    TcpBuffer &m_in_buffer;         // TcpConnection中的m_in_buffer的引用
    std::size_t m_remain_len {0};   // 当前请求还需要读取的字节数
    MainState m_state {STATE_WAITING_START_FLAG};
};

    
} // namespace zest


#endif