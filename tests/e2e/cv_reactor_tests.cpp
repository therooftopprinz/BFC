#include <gtest/gtest.h>
#include <bfc/cv_reactor.hpp>

#include <thread>

using namespace bfc;

struct fake_reactor
{
    void wake_up(){};
};

using r_cb_t = std::function<void()>;
using queue_t = cv_event_queue<uint64_t, fake_reactor, r_cb_t>;

struct counters_t
{
    uint64_t server_read = 0;
    uint64_t server_write = 0;
    uint64_t client_read = 0;
    uint64_t client_write = 0;
};

constexpr uint64_t N = 5000000;

TEST(cv_reactor, native_st)
{
    std::list<uint64_t> q;

    auto t_start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    for (uint64_t i=0; i<N; i++)
    {
        q.push_front(i);
        auto j = q.back();
        q.pop_back();
        ASSERT_EQ(i,j);
    }
    auto t_end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    
    auto t_diff = (t_end - t_start);
    auto tput = double(N) * 1000 * 1000 * 1000 / t_diff;

    printf("ms_diff: %lf\n", double(t_diff)/1000000);
    printf("tput: %lf\n", tput);
}
 

TEST(cv_reactor, native_mt)
{
    std::mutex m;
    std::list<uint64_t> q;

    std::thread writer = std::thread([&q, &m](){
            for (uint64_t i=0; i<N; i++)
            {
                std::unique_lock l(m);
                q.push_back(i);
            }
        });

    auto t_start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    uint64_t n = 0;
    while (n < N)
    {
        std::unique_lock l(m);
        n += q.size();
        q.clear();
    }
    auto t_end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    writer.join();

    auto t_diff = (t_end - t_start);
    auto tput = double(N) * 1000 * 1000 * 1000 / t_diff;

    printf("ms_diff: %lf\n", double(t_diff)/1000000);
    printf("tput: %lf\n", tput);
}


TEST(cv_reactor, non_reactive_st)
{
    queue_t q;

    auto t_start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    for (uint64_t i=0; i<N; i++)
    {
        q.push(i);
        q.pop();
    }
    auto t_end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    
    auto t_diff = (t_end - t_start);
    auto tput = double(N) * 1000 * 1000 * 1000 / t_diff;

    printf("ms_diff: %lf\n", double(t_diff)/1000000);
    printf("tput: %lf\n", tput);
}
 

TEST(cv_reactor, non_reactive_mt)
{
    queue_t q;

    std::thread writer = std::thread([&q](){
            for (uint64_t i=0; i<N; i++)
            {
                q.push(i);
            }
        });

    auto t_start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    uint64_t n = 0;
    while (n < N)
    {
        auto r = q.pop();
        n += r.size();
    }
    auto t_end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    writer.join();

    auto t_diff = (t_end - t_start);
    auto tput = double(N) * 1000 * 1000 * 1000 / t_diff;

    printf("ms_diff: %lf\n", double(t_diff)/1000000);
    printf("tput: %lf\n", tput);
}
 