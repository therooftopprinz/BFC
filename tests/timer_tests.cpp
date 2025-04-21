// #include <thread>

// #include <gtest/gtest.h>
// #include <gmock/gmock.h>

// #include <bfc/timer.hpp>

// using namespace bfc;
// using namespace testing;

// struct SequenceChecker
// {
//     MOCK_METHOD1(check, void(int));
// };

// struct TimerTest : Test
// {
//     void SetUp()
//     {
//         runner = std::thread([this](){sut.run();});
//     }

//     void TearDown()
//     {
//         sut.stop();
//         runner.join();
//     }

//     void stopTest()
//     {
//         std::unique_lock<std::mutex> lg(endOfTestMutex);
//         endOfTest = true;
//         endOfTesCv.notify_one();
//     }

//     void waitTest()
//     {
//         std::unique_lock<std::mutex> lg(endOfTestMutex);
//         endOfTesCv.wait(lg, [this](){return endOfTest;});
//     }

//     timer<> sut;
//     std::thread runner;
//     SequenceChecker checker;

//     std::vector<int> checker2;
//     std::mutex checker2mutex;

//     bool endOfTest = false;
//     std::mutex endOfTestMutex;
//     std::condition_variable endOfTesCv;

//     static constexpr uint64_t FACTOR = 1000;
    
// };

// TEST_F(TimerTest, shouldSchedule)
// {
//     sut.wait_ms(std::chrono::nanoseconds(1000*1000*1), [this](){sut.stop();});
// }

// TEST_F(TimerTest, shouldScheduleEarlier)
// {
//     InSequence seq;
//     EXPECT_CALL(checker, check(1));
//     EXPECT_CALL(checker, check(200));

//     sut.wait_ms(std::chrono::nanoseconds(FACTOR*200), [this]()
//         {
//             checker.check(200);
//             stopTest();
//         });
//     sut.wait_ms(std::chrono::nanoseconds(FACTOR*1), [this]()
//         {
//             checker.check(1);
//         });
//     waitTest();
// }

// TEST_F(TimerTest, shouldCancel)
// {
//     constexpr uint64_t BASE = FACTOR*1;
//     std::vector<int> expectedSequence;

//     for (int i = 0; i<50; i++)
//     {
//         if (0 != i%5 || 0 == i%10 || i == 0)
//         {
//             expectedSequence.emplace_back(i);
//         }

//         // std::cout << "SCHEDULE " << i << "\n";
//         sut.wait_ms(std::chrono::nanoseconds(BASE + FACTOR*i*1000), [this, i]()
//             {
//                 // std::cout << "RUN " << i << "\n";
//                 {
//                     std::unique_lock<std::mutex> lg(checker2mutex);
//                     checker2.emplace_back(i);
//                 }
//                 if (0 == i%10)
//                 {
//                     auto id = i + 5;
//                     sut.cancel(id);
//                     // std::cout << "TRY CANCEL " << id << "\n";
//                 }
//                 if (i==49)
//                 {
//                     stopTest();
//                 }
//             });
//     }

//     waitTest();

//     EXPECT_THAT(checker2, ElementsAreArray(expectedSequence));
// }