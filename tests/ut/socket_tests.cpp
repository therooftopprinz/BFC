#include <gtest/gtest.h>

#include <bfc/socket.hpp>

using namespace bfc;

TEST(socket, shouldGenerateAndParseIPv4)
{
    auto addr = ip4_port_to_sockaddr("127.0.0.1", 1234);
    auto addr_str = sockaddr_to_string((sockaddr*) &addr);
    EXPECT_EQ("127.0.0.1:1234", addr_str);
}

TEST(socket, shouldGenerateAndParse2IPv4)
{
    auto addr = ip4_port_to_sockaddr(to_ip(127,0,0,1), 1234);
    auto addr_str = sockaddr_to_string((sockaddr*) &addr);
    EXPECT_EQ("127.0.0.1:1234", addr_str);
}
