#include <gtest/gtest.h>

#include <bfc/ThreadPool.hpp>
#include <bfc/FixedFunctionObject.hpp>

using namespace bfc;

TEST(ThreadPool, ShouldExecute)
{
    ThreadPool pool;

    std::promise<int> res1;
    std::promise<int> res2;
    std::promise<int> res3;
    std::promise<int> res4;
    std::promise<int> res5;

    ThreadPool::Functor exec1 = [&res1](){res1.set_value(42);};
    ThreadPool::Functor exec2 = [&res2](){res2.set_value(43);};
    ThreadPool::Functor exec3 = [&res3](){res3.set_value(44);};
    ThreadPool::Functor exec4 = [&res4](){res4.set_value(45);};
    ThreadPool::Functor exec5 = [&res5](){res5.set_value(46);};

    pool.execute(exec1);
    pool.execute(exec2);
    pool.execute(exec3);
    pool.execute(exec4);
    pool.execute(exec5);

    EXPECT_EQ(42, res1.get_future().get());
    EXPECT_EQ(43, res2.get_future().get());
    EXPECT_EQ(44, res3.get_future().get());
    EXPECT_EQ(45, res4.get_future().get());
    EXPECT_EQ(46, res5.get_future().get());
}
