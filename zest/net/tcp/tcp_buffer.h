/* 对 std::string 进行一些封装，旨在降低substr()调用次数，提高效率 */
#ifndef ZEST_NET_TCP_TCP_BUFFER
#define ZEST_NET_TCP_TCP_BUFFER
#include <string>


namespace zest
{

class TcpBuffer
{
public:

    friend void swap(TcpBuffer &buf1, TcpBuffer &buf2);

    char& operator[](std::size_t index) {return m_buffer[m_start_index+index];}

    TcpBuffer& operator+=(const std::string &s)
    {
        shrink();
        m_buffer += s;
        return *this;
    }

    TcpBuffer operator+(const std::string &s)
    {
        TcpBuffer tmp_buf = *this;
        tmp_buf += s;
        return tmp_buf;
    }

    TcpBuffer& operator+=(const char *s)
    {
        shrink();
        m_buffer += s;
        return *this;
    }
    
    TcpBuffer operator+(const char *s)
    {
        TcpBuffer tmp_buf = *this;
        tmp_buf += s;
        return tmp_buf;
    }

    std::size_t size() const {return m_buffer.size()-m_start_index;}

    void clear() 
    {
        m_buffer.clear();
        m_start_index = 0;
    }

    bool empty() const {return m_start_index >= m_buffer.size();}

    const char* c_str()
    {
        shrink();
        return m_buffer.c_str();
    }

    std::string substr(std::size_t pos = 0, std::size_t n = 0)
    {
        if (n != 0)
            return m_buffer.substr(m_start_index+pos, n);
        else
            return m_buffer.substr(m_start_index+pos);
    }

    void move_forward(std::size_t size)
    {
        m_start_index += size;

        // 当 m_buffer 中没用的数据超过1/3,则调用shrink清楚无用数据
        if (m_start_index * 3 > m_buffer.size()) {
            shrink();
        }
    }

private:
    // 清除无用的数据，减少内存使用量。由TcpBuffer自动完成，用户代码不需要关心
    void shrink()
    {
        if (m_start_index > 0) {
            m_buffer = m_buffer.substr(m_start_index);
            m_start_index = 0;
        }
    }   

private:
    std::string m_buffer;
    std::size_t m_start_index {0};
};

// 交换两个TcpBuffer
void swap(TcpBuffer &buf1, TcpBuffer &buf2)
{
    using std::swap;
    swap(buf1.m_buffer, buf2.m_buffer);
    swap(buf1.m_start_index, buf2.m_start_index);
}

} // namespace zest

#endif