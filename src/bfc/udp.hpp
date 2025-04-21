
#ifndef __BFC_UDP_HPP__
#define __BFC_UDP_HPP__

#include <bfc/socket.hpp>

namespace bfc
{


class udp_socket : public socket
{
public:
    udp_socket()
        : socket(::socket(AF_INET, SOCK_DGRAM, 0))
    {}

    ~udp_socket()
    {}
};


} // namespace bfc

#endif // __BFC_UDP_HPP__
