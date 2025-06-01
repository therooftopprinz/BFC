#include <gtest/gtest.h>
#include <bfc/epoll_reactor.hpp>
#include <bfc/socket.hpp>

// #undef ASSERT_NE
// #define ASSERT_NE(A, B) B

using namespace bfc;

using r_cb_t = std::function<void()>;
using reactor_t = bfc::epoll_reactor<r_cb_t>;

struct counters_t
{
    uint64_t server_read = 0;
    uint64_t server_write = 0;
    uint64_t client_read = 0;
    uint64_t client_write = 0;
};

constexpr uint64_t N = 500000;

TEST(epoll_reactor, non_reactive_st)
{
    bfc::socket acceptor(create_tcp4());
    bfc::socket client(create_tcp4());
    bfc::socket server;

    acceptor.set_sock_opt(SOL_SOCKET , SO_REUSEADDR, 1);
    ASSERT_NE(-1, acceptor.bind(ip4_port_to_sockaddr(localhost4, 12345)));
    ASSERT_NE(-1, acceptor.listen());

    ASSERT_NE(-1, client.connect(ip4_port_to_sockaddr(localhost4, 12345)));
    printf("client: connected!\n");

    server = std::move(acceptor.accept(nullptr, nullptr));
    printf("server: accepted!\n");

    auto t_start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    for (uint64_t i=0; i < N; i++)
    {
        {
            uint64_t b = i;
            buffer_view wb((std::byte*) &b, sizeof(b));
            ASSERT_NE(-1, client.send(wb, 0));
        }
        {
            uint64_t b;
            buffer_view rb((std::byte*) &b, sizeof(b));
            ASSERT_NE(-1, server.recv(rb, 0));
        }
    }
    auto t_end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    
    auto t_diff = (t_end - t_start);
    auto tput = double(N) * 1000 * 1000 * 1000 / t_diff;
    tput /= 1000000;

    printf("tput: %lf\n", tput);
}

TEST(epoll_reactor, non_reactive_mt)
{
    bfc::socket acceptor(create_tcp4());
    bfc::socket client(create_tcp4());
    bfc::socket server;

    acceptor.set_sock_opt(SOL_SOCKET , SO_REUSEADDR, 1);
    ASSERT_NE(-1, acceptor.bind(ip4_port_to_sockaddr(localhost4, 12345)));
    ASSERT_NE(-1, acceptor.listen());

    ASSERT_NE(-1, client.connect(ip4_port_to_sockaddr(localhost4, 12345)));
    printf("client: connected!\n");

    server = std::move(acceptor.accept(nullptr, nullptr));
    printf("server: accepted!\n");

    std::thread sender = std::thread([&](){
        for (uint64_t i=0; i < N; i++)
        {
            uint64_t b = i;
            buffer_view wb((std::byte*) &b, sizeof(b));
            ASSERT_NE(-1, client.send(wb, 0));
        }
    });

    auto t_start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    for (uint64_t i=0; i < N; i++)
    {
        uint64_t b;
        buffer_view rb((std::byte*) &b, sizeof(b));
        ASSERT_NE(-1, server.recv(rb, 0));
    }
    auto t_end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    sender.join();
    
    auto t_diff = (t_end - t_start);
    auto tput = double(N) * 1000 * 1000 * 1000 / t_diff;
    tput /= 1000000;

    printf("tput: %lf\n", tput);
}

TEST(epoll_reactor, reactive_read)
{
    reactor_t reactor;
    bfc::socket acceptor(create_tcp4());
    bfc::socket client(create_tcp4());
    bfc::socket server;

    acceptor.set_sock_opt(SOL_SOCKET , SO_REUSEADDR, 1);
    ASSERT_NE(-1, acceptor.bind(ip4_port_to_sockaddr(localhost4, 12345)));
    ASSERT_NE(-1, acceptor.listen());

    ASSERT_NE(-1, client.connect(ip4_port_to_sockaddr(localhost4, 12345)));
    printf("client: connected!\n");

    server = std::move(acceptor.accept(nullptr, nullptr));
    printf("server: accepted!\n");

    std::thread sender = std::thread([&](){
        for (uint64_t i=0; i < N; i++)
        {
            uint64_t b = i;
            buffer_view wb((std::byte*) &b, sizeof(b));
            ASSERT_NE(-1, client.send(wb, 0));
        }
    });

    auto server_ctx = reactor.make_context(server.fd());

    ASSERT_NE(-1, reactor.add_read_rdy(server_ctx, [&reactor, &server](){
            uint64_t b;
            buffer_view rb((std::byte*) &b, sizeof(b));
            ASSERT_NE(-1, server.recv(rb, 0));
            if (b >= N-1)
            {
                reactor.stop();
            }
        }));

    auto t_start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    reactor.run();
    auto t_end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    sender.join();
    
    auto t_diff = (t_end - t_start);
    auto tput = double(N) * 1000 * 1000 * 1000 / t_diff;
    tput /= 1000000;

    printf("tput: %lf\n", tput);
}

TEST(epoll_reactor, reactive_write)
{
    reactor_t reactor;
    bfc::socket acceptor(create_tcp4());
    bfc::socket client(create_tcp4());
    bfc::socket server;

    acceptor.set_sock_opt(SOL_SOCKET , SO_REUSEADDR, 1);
    ASSERT_NE(-1, acceptor.bind(ip4_port_to_sockaddr(localhost4, 12345)));
    ASSERT_NE(-1, acceptor.listen());

    ASSERT_NE(-1, client.connect(ip4_port_to_sockaddr(localhost4, 12345)));
    printf("client: connected!\n");

    server = std::move(acceptor.accept(nullptr, nullptr));
    printf("server: accepted!\n");

    std::thread receiver = std::thread([&](){
        for (uint64_t i=0; i < N; i++)
        {
            uint64_t b = i;
            buffer_view wb((std::byte*) &b, sizeof(b));
            ASSERT_NE(-1, server.recv(wb, 0));
        }
        reactor.stop();
    });

    auto client_context = reactor.make_context(client.fd());

    uint64_t i=0;
    r_cb_t on_client_write_rdy = [&reactor, &client_context, &client, &i](){
            uint64_t b = i;
            buffer_view wb((std::byte*) &b, sizeof(b));
            ASSERT_NE(-1, client.send(wb, 0));
            i++;
            if (i >= N)
            {
                // printf("client: stop!\n");
                return;
            }
            ASSERT_NE(false, reactor.req_write(client_context));
        };

    reactor.add_write_rdy(client_context, on_client_write_rdy);
    reactor.req_write(client_context);

    auto t_start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    reactor.run();
    auto t_end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    receiver.join();
    
    auto t_diff = (t_end - t_start);
    auto tput = double(N) * 1000 * 1000 * 1000 / t_diff;
    tput /= 1000000;

    printf("tput: %lf\n", tput);
}

TEST(epoll_reactor, reactive)
{
    reactor_t reactor;
    bfc::socket acceptor(create_tcp4());
    bfc::socket client(create_tcp4());
    bfc::socket server;

    counters_t ctrs;

    acceptor.set_sock_opt(SOL_SOCKET , SO_REUSEADDR, 1);
    ASSERT_NE(-1, acceptor.bind(ip4_port_to_sockaddr(localhost4, 12345)));
    ASSERT_NE(-1, acceptor.listen());

    r_cb_t on_server_read_rdy = [&reactor, &server, &ctrs](){
            uint64_t b;
            buffer_view rb((std::byte*) &b, sizeof(b));
            ASSERT_NE(-1, server.recv(rb, 0));
            // printf("server: read! v=%zu\n", b);
            ctrs.server_read++;
            if (ctrs.server_read >= N)
            {
                reactor.stop();
            }
        };

    auto client_context = reactor.make_context(client.fd());

    r_cb_t on_client_write_rdy = [&reactor, &client, &client_context, &ctrs](){
            uint64_t b = ctrs.client_write;
            buffer_view wb((std::byte*) &b, sizeof(b));
            ASSERT_NE(-1, client.send(wb, 0));
            // printf("client: write! v=%zu\n", ctrs.client_write);
            ctrs.client_write++;
            if (ctrs.client_write >= N)
            {
                // printf("client: stop!\n");
                return;
            }

            // printf("client: wreq!\n");
            ASSERT_NE(-1, reactor.req_write(client_context));
        };

    ASSERT_NE(-1, client.connect(ip4_port_to_sockaddr(localhost4, 12345)));
    printf("client: connected!\n");
    server = std::move(acceptor.accept(nullptr, nullptr));
    printf("server: accepted!\n");
    ASSERT_NE(-1, server.fd());

    auto server_context = reactor.make_context(server.fd());
    ASSERT_NE(false, reactor.add_read_rdy(server_context, on_server_read_rdy));

    reactor.add_write_rdy(client_context, on_client_write_rdy);
    reactor.req_write(client_context);

    auto t_start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    reactor.run();
    auto t_end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    
    auto t_diff = (t_end - t_start);
    auto tput = double(N) * 1000 * 1000 * 1000 / t_diff;
    tput /= 1000000;

    printf("counters.send: %zu\n", ctrs.client_write);
    printf("counters.recv: %zu\n", ctrs.server_read);
    printf("tput: %lf\n", tput);
}
