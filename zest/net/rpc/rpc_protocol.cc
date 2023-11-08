/* RPC协议的封装，用有限状态机作为解码器解析协议，以及编码器函数 */
#include "zest/net/rpc/rpc_protocol.h"
#include "zest/common/logging.h"
#include "zest/common/util.h"
#include <string.h>


namespace zest
{

RpcDecoder::RpcDecoder(TcpBuffer &in_buffer) :
    m_req_protocol(new RpcProtocol()), m_in_buffer(in_buffer)
{
    /* do nothing */
}

// 解析 m_in_buffer 序列，返回 RpcProtocol 指针
// 如果返回空指针，则需要通过状态判断是出错还是未收到完整的请求
RpcProtocol::s_ptr RpcDecoder::decode()
{
    if (m_state == STATE_PARSE_SUCCESS || m_state == STATE_PARSE_FAILED) {
        clear();
        m_state = STATE_WAITING_START_FLAG;
    }

    if (m_state == STATE_WAITING_START_FLAG) {
        if (m_in_buffer.size() < 1) return nullptr;
        if (m_in_buffer[0] == RpcProtocol::START_FLAG) {
            m_in_buffer.move_forward(1);
            m_state = STATE_PARSE_PK_LEN;
        }
        else {
            m_state = STATE_PARSE_FAILED;
            return nullptr;
        }
    }

    if (m_state == STATE_PARSE_PK_LEN) {
        if (m_in_buffer.size() < 4) return nullptr;
        m_req_protocol->m_pk_len = get_int32_from_net_bytes(&m_in_buffer[0]);
        m_remain_len = m_req_protocol->m_pk_len - 5;
        m_in_buffer.move_forward(4);
        m_state = STATE_PARSE_MSG_ID_LEN;
    }

    if (m_state == STATE_PARSE_MSG_ID_LEN) {
        if (m_in_buffer.size() < 4) return nullptr;
        m_req_protocol->m_msg_id_len = get_int32_from_net_bytes(&m_in_buffer[0]);
        m_remain_len -= 4;
        m_in_buffer.move_forward(4);
        m_state = STATE_PARSE_MSG_ID;
    }

    if (m_state == STATE_PARSE_MSG_ID) {
        int32_t len = m_req_protocol->m_msg_id_len;
        if (m_in_buffer.size() < len) return nullptr;
        m_req_protocol->m_msg_id = m_in_buffer.substr(0, len);
        m_remain_len -= len;
        m_in_buffer.move_forward(len);
        m_state = STATE_PARSE_METHOD_NAME_LEN;
    }

    if (m_state == STATE_PARSE_METHOD_NAME_LEN) {
        if (m_in_buffer.size() < 4) return nullptr;
        m_req_protocol->m_method_name_len = get_int32_from_net_bytes(&m_in_buffer[0]);
        m_remain_len -= 4;
        m_in_buffer.move_forward(4);
        m_state = STATE_PARSE_METHOD_NAME;
    }

    if (m_state == STATE_PARSE_METHOD_NAME) {
        int32_t len = m_req_protocol->m_method_name_len;
        if (m_in_buffer.size() < len) return nullptr;
        m_req_protocol->m_method_name = m_in_buffer.substr(0, len);
        m_remain_len -= len;
        m_in_buffer.move_forward(len);
        m_state = STATE_PARSE_ERROR_CODE;
    }

    if (m_state == STATE_PARSE_ERROR_CODE) {
        if (m_in_buffer.size() < 4) return nullptr;
        m_req_protocol->m_error_code = get_int32_from_net_bytes(&m_in_buffer[0]);
        m_remain_len -= 4;
        m_in_buffer.move_forward(4);
        m_state = STATE_PARSE_ERROR_INFO_LEN;
    }

    if (m_state == STATE_PARSE_ERROR_INFO_LEN) {
        if (m_in_buffer.size() < 4) return nullptr;
        m_req_protocol->m_error_info_len = get_int32_from_net_bytes(&m_in_buffer[0]);
        m_remain_len -= 4;
        m_in_buffer.move_forward(4);
        m_state = STATE_PARSE_ERROR_INFO;
    }

    if (m_state == STATE_PARSE_ERROR_INFO) {
        int32_t len = m_req_protocol->m_error_info_len;
        if (m_in_buffer.size() < len) return nullptr;
        m_req_protocol->m_error_info = m_in_buffer.substr(0, len);
        m_remain_len -= len;
        m_in_buffer.move_forward(len);
        m_state = STATE_PARSE_RPC_DATA;
    }

    if (m_state == STATE_PARSE_RPC_DATA) {
        int32_t len = m_remain_len - 5;
        if (m_in_buffer.size() < len) return nullptr;
        m_req_protocol->m_rpc_data = m_in_buffer.substr(0, len);
        m_remain_len -= len;
        m_in_buffer.move_forward(len);
        m_state = STATE_PARSE_CHECK_SUM;
    }

    if (m_state == STATE_PARSE_CHECK_SUM) {
        if (m_in_buffer.size() < 4) return nullptr;
        m_req_protocol->m_check_sum = get_int32_from_net_bytes(&m_in_buffer[0]);
        m_remain_len -= 4;
        m_in_buffer.move_forward(4);
        m_state = STATE_WAITING_END_FLAG;
    }

    if (m_state == STATE_WAITING_END_FLAG) {
        if (m_in_buffer.size() < 1) return nullptr;
        if (m_in_buffer[0] == RpcProtocol::END_FLAG) {
            m_in_buffer.move_forward(1);
            m_state = STATE_VALID_CHECK_SUM;
        }
        else {
            m_state = STATE_PARSE_FAILED;
            return nullptr;
        }
    }

    if (m_state == STATE_VALID_CHECK_SUM) {
        if (valid_check_sum()) {
            m_state = STATE_PARSE_SUCCESS;
        }
        else {
            m_state = STATE_PARSE_FAILED;
            return nullptr;
        }
    }

    if (m_state == STATE_PARSE_SUCCESS) {
        return m_req_protocol;
    }

    return nullptr;
}

void RpcDecoder::clear()
{
    m_req_protocol->m_pk_len = 0;
    m_req_protocol->m_msg_id_len = 0;
    m_req_protocol->m_msg_id.clear();
    m_req_protocol->m_method_name_len = 0;
    m_req_protocol->m_method_name.clear();
    m_req_protocol->m_error_code = 0;
    m_req_protocol->m_error_info_len = 0;
    m_req_protocol->m_error_info.clear();
    m_req_protocol->m_rpc_data.clear();
    m_req_protocol->m_check_sum = 0;
}

bool RpcDecoder::valid_check_sum() const
{
    // TODO: 暂时还没写检验检验和，只是简单地返回true
    return true;
}
    
} // namespace zest
