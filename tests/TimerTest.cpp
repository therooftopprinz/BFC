#include <thread>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <bfc/Timer.hpp>

using namespace bfc;
using namespace testing;

struct SequenceChecker
{
    MOCK_METHOD1(check, void(int));
};

struct TimerTest : Test
{
    void SetUp()
    {
        runner = std::thread([this](){sut.run();});
    }

    void TearDown()
    {
        runner.join();
    }
    Timer<> sut;
    std::thread runner;
    SequenceChecker checker;
};

TEST_F(TimerTest, shouldSchedule)
{
    sut.schedule(std::chrono::nanoseconds(1000*1000*1), [this](){sut.stop();});
}

TEST_F(TimerTest, shouldScheduleEarlier)
{
    InSequence seq;
    EXPECT_CALL(checker, check(1));
    EXPECT_CALL(checker, check(20));

    sut.schedule(std::chrono::nanoseconds(1000*1000*20), [this]()
        {
            checker.check(20);
            sut.stop();
        });
    sut.schedule(std::chrono::nanoseconds(1000*1000*1), [this]()
        {
            checker.check(1);
        });
}

TEST_F(TimerTest, shouldCancel)
{
    InSequence seq;
    EXPECT_CALL(checker, check(20));
    EXPECT_CALL(checker, check(30));
    // EXPECT_CALL(checker, check(40));
    EXPECT_CALL(checker, check(50));

    sut.schedule(std::chrono::nanoseconds(1000*1000*50), [this]()
        {
            checker.check(50);
            sut.stop();
        });

    sut.schedule(std::chrono::nanoseconds(1000*1000*40), [this]()
        {
            checker.check(40);
        });

    sut.schedule(std::chrono::nanoseconds(1000*1000*30), [this]()
        {
            checker.check(30);
        });

    sut.schedule(std::chrono::nanoseconds(1000*1000*20), [this]()
        {
            checker.check(20);
            sut.cancel(1);
        });
}