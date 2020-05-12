#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include <map>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <iostream>

#include <bfc/FixedFunctionObject.hpp>
#include <bfc/ThreadPool.hpp>

namespace bfc
{

template <typename FunctorType = LightFn<void()>>
class Timer
{
public:
    using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>; 
    Timer()
    {
        mStartRef = std::chrono::high_resolution_clock::now();
    }

    int schedule(std::chrono::nanoseconds pDiff, FunctorType pCb)
    {
        auto tp = std::chrono::high_resolution_clock::now();
        std::unique_lock<std::mutex> lg(mSchedulesMutex);
        mSchedulesCv.notify_one();
        auto id = mIdCtr++;
        auto next = tp + pDiff;

        [[maybe_unused]] auto printTp = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count();
        [[maybe_unused]] auto printNx = std::chrono::duration_cast<std::chrono::nanoseconds>(next.time_since_epoch()).count();
        [[maybe_unused]] auto printDf = std::chrono::duration_cast<std::chrono::nanoseconds>(pDiff).count();
        // std::cout << "ADD(" << id << ") TPA=" << printTp << " DIFF=" << printDf << " NX=" << printNx << "\n";

        auto res = mSchedules.emplace(next, std::pair(id, std::move(pCb)));
        mScheduleIds.emplace(id, res);
        mAddRequest = true;
        lg.unlock();
        return id;
    }

    void cancel(int pId)
    {
        std::unique_lock<std::mutex> lg(mSchedulesMutex);
        mSchedulesCv.notify_one();
        auto foundIt = mScheduleIds.find(pId);
        if (mScheduleIds.end() == foundIt)
        {
            return;
        }
        // std::cout << "CANCEL " << pId << "\n";
        auto remIt = foundIt->second;
        mScheduleIds.erase(foundIt);
        mSchedules.erase(remIt);
        mCancelRequest = true;
    }

    void run()
    {
        mRunning = true;
        auto waitBreaker = [this]() {
                return !mRunning || mCancelRequest || mAddRequest;
            };

        while (mRunning)
        {
            if (mToRun && !mCancelRequest && !mAddRequest)
            {
                // std::cout << "RUN " << mToRunId << "\n";
                mToRun();
                mToRun.reset();
            }

            std::unique_lock<std::mutex> lg(mSchedulesMutex);
            auto tp = std::chrono::high_resolution_clock::now();

            if (mSchedules.size())
            {
                auto firstIt =  mSchedules.begin();

                auto setToRun = [this](auto firstIt)
                {
                    mToRunTime = firstIt->first;
                    mToRunId = firstIt->second.first;
                    mToRun = std::move(firstIt->second.second);
                    mScheduleIds.erase(firstIt->second.first);
                    mSchedules.erase(firstIt);
                };

                if (mToRun && mToRunTime > firstIt->first)
                {
                    auto res = mSchedules.emplace(mToRunTime, std::pair(mToRunId, std::move(mToRun)));
                    mScheduleIds.emplace(mToRunId, res);
                    setToRun(firstIt);
                }
                else if(!mToRun)
                {
                    setToRun(firstIt);
                }

                auto tdiff = mToRunTime - tp;
                mAddRequest = false;
                mCancelRequest = false;
                mSchedulesCv.wait_for(lg, tdiff, waitBreaker);

                [[maybe_unused]] auto ntp = std::chrono::high_resolution_clock::now();
                [[maybe_unused]] auto printTp = std::chrono::duration_cast<std::chrono::nanoseconds>(ntp.time_since_epoch()).count();
                [[maybe_unused]] auto printAwt = std::chrono::duration_cast<std::chrono::nanoseconds>(ntp-tp).count();
                [[maybe_unused]] auto printIwt = std::chrono::duration_cast<std::chrono::nanoseconds>(tdiff).count();
                // std::cout << "BRK TPR=" << printTp << " IWT=" << printIwt << " AWT=" << printAwt << "\n";
                continue;
            }
            else
            {
                mToRun.reset();
                mCancelRequest = false;
            }
            
            mSchedulesCv.wait(lg, waitBreaker);
        }
    }

    void stop()
    {
        mRunning = false;
        mSchedulesCv.notify_one();
    }

private:
    using ScheduleIdFn = std::pair<int, FunctorType>;
    using ScheduleMap = std::multimap<TimePoint, ScheduleIdFn>;
    ScheduleMap mSchedules;
    std::unordered_map<int, typename ScheduleMap::iterator> mScheduleIds;
    int mIdCtr = 0;

    FunctorType mToRun;
    int mToRunId;
    TimePoint mToRunTime;
    
    std::condition_variable mSchedulesCv;
    bool mCancelRequest;
    bool mAddRequest;
    std::mutex mSchedulesMutex;

    TimePoint mStartRef;
    bool mRunning;
};
} // namespace bfc

#endif // __TIMER_HPP__
