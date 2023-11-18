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

    // 各种错误码
    static const int32_t ERROR_NONE               = 0x00000000;  // 正常，没有出错
    static const int32_t ERROR_PARSE_METHOD_NAME  = 0x00000001;  // 解析service和method名称错误
    static const int32_t ERROR_SERVICE_NOT_FOUND  = 0x00000002;  // service 不存在
    static const int32_t ERROR_SERIALIZE_FAILED   = 0x00000003;  // 序列化错误
    static const int32_t ERROR_DESERIALIZE_FAILED = 0x00000004;  // 反序列化错误
    static const int32_t ERROR_BAD_REQUEST        = 0x00000005;  // 请求的格式错误或者校验和失败
    static const int32_t ERROR_CONNECT_FAILED     = 0x00000006;  // 客户端连接失败
    static const int32_t ERROR_CALL_TIMEOUT       = 0x00000007;  // RPC调用超时
    static const int32_t ERROR_CALL_FAILED        = 0x00000008;  // RPC调用失败（服务器断开连接，或者无法解析服务器发来的字节序列）
    
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

    void clear();
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

    // 返回主状态机的状态
    int get_state() const {return m_state;}

private:
    void clear();
    bool valid_check_sum() const;
    
private:
    RpcProtocol::s_ptr m_req_protocol;
    TcpBuffer &m_in_buffer;         // TcpConnection中的m_in_buffer的引用
    std::size_t m_remain_len {0};   // 当前请求还需要读取的字节数
    MainState m_state {STATE_WAITING_START_FLAG};
};


// 将RPC协议结构体编码成字节序列
void RpcEncoder(RpcProtocol::s_ptr rpc_protocol, TcpBuffer &out_buffer);

// 计算校验和并填充
void fill_check_sum(RpcProtocol::s_ptr protocol);

} // namespace zest


#endif