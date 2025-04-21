#ifndef __BFC_SOCKET_HPP__
#define __BFC_SOCKET_HPP__

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <memory>
#include <string>
#include <stdexcept>

#include <bfc/buffer.hpp>

namespace bfc
{


uint32_t to_ip(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    uint32_t rv = a;
    rv << 8; rv |= b;
    rv << 8; rv |= c;
    rv << 8; rv |= d;
    return rv;
}

sockaddr_in to_ip_port(uint32_t ip, uint16_t port)
{
    sockaddr_in addr;
    memset((char *)&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(ip);
    addr.sin_port = htons(port);
    return addr;
}

class socket
{
public:
    socket(int p_fd)
        : m_fd(p_fd)
    {}

    virtual ~socket()
    {
        if (-1 != m_fd)
        {
            close (m_fd);
        }
    }

    socket(const socket&) = delete;
    socket(const socket&&) = delete;

    socket(socket&& p_other)
    {
        if (-1 != p_other.m_fd)
        {
            m_fd = p_other.m_fd;
            p_other.m_fd = -1;
        }
    }

    void operator=(const socket&& p_other) = delete;
    void operator=(socket&& p_other)
    {
        if (-1 != p_other.m_fd)
        {
            if (-1 !=m_fd)
            {
                close(m_fd);
            }

            m_fd = p_other.m_fd;
            p_other.m_fd = -1;
        }
    }


    int bind(const sockaddr *p_addr, socklen_t p_size)
    {
        return ::bind(m_fd, p_addr, p_size);
    }

    ssize_t send(const bfc::const_buffer_view& p_data, int p_flags, const sockaddr* p_to, socklen_t p_to_size)
    {
        return sendto(m_fd, p_data.data(), p_data.size(), p_flags, p_to, p_to_size);
    }

    ssize_t recv(bfc::buffer_view& p_data, int p_flags, sockaddr* p_addr, socklen_t* p_addr_sz)
    {
        return recvfrom(m_fd, p_data.data(), p_data.size(), p_flags, p_addr, p_addr_sz);
    }

    ssize_t send(const bfc::const_buffer_view& p_data, int p_flags)
    {
        return ::send(m_fd, p_data.data(), p_data.size(), p_flags);
    }

    ssize_t recv(bfc::buffer_view& p_data, int p_flags)
    {
        return ::recv(m_fd, p_data.data(), p_data.size(), p_flags);
    }

    int set_sock_opt(int p_level, int p_name, const void *p_value, socklen_t p_len)
    {
        return setsockopt(m_fd, p_level, p_name, p_value, p_len);
    }

    int listen(int p_size)
    {
        return ::listen(m_fd, p_size);
    }

    int accept(sockaddr* p_addr, socklen_t* p_addr_sz)
    {
        return ::accept(m_fd, p_addr, p_addr_sz);
    }

    int fd()
    {
        return m_fd;
    }

protected:
    int m_fd = -1;
};


} // namespace bfc

#endif // __BFC_SOCKET_HPP__
