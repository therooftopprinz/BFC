#include <gtest/gtest.h>

#include <bfc/ThreadPool.hpp>
#include <bfc/FixedFunctionObject.hpp>

using namespace bfc;

TEST(ThreadPool, ShouldExecute)
{
    using TP = ThreadPool<LightFn<void()>>;
    TP pool;

    constexpr int COUNT = 50;
    std::vector<std::promise<int>> res(COUNT);
    std::vector<TP::Functor> exec;

    for (int i=0u; i<COUNT; i++)
    {
        exec.emplace_back([&res, i](){res[i].set_value(i);});
    }

    for (auto& i : exec)
    {
        pool.execute(i);
        std::printf("Active %lu\n", pool.countActive());
    }

    for (int i=0u; i<COUNT; i++)
    {
        EXPECT_EQ(i, res[i].get_future().get());
    }
    std::printf("Pool size %lu\n", pool.size());

}
