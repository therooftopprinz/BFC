#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include <map>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include <bfc/FixedFunctionObject.hpp>
#include <bfc/ThreadPool.hpp>

namespace bfc
{

template <typename FunctorType = LightFn<void()>>
class Timer
{
public:
    using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
    Timer() = default;

    int schedule(std::chrono::nanoseconds pDiff, FunctorType pCb)
    {
        auto tp = std::chrono::high_resolution_clock::now();
        std::unique_lock<std::mutex> lg(mSchedulesMutex);
        mSchedulesCv.notify_one();
        auto id = mIdCtr++;
        auto res = mSchedules.emplace(tp + pDiff, std::pair(id, std::move(pCb)));
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
        auto remIt = foundIt->second;
        mScheduleIds.erase(foundIt);
        mSchedules.erase(remIt);
        mCancelRequest = true;
    }

    void run()
    {
        mRunning = true;
        auto waitBreaker = [this]() {
                bool hasCancel = mCancelRequest;
                bool hasAdd = mAddRequest;
                mCancelRequest = false;
                mAddRequest = false;
                return !mRunning || hasCancel || hasAdd;
            };

        while (mRunning)
        {
            if (mToRun)
            {
                mToRun();
                mToRun.reset();
            }

            std::unique_lock<std::mutex> lg(mSchedulesMutex);
            auto tp = std::chrono::high_resolution_clock::now();

            if (mSchedules.size())
            {
                auto firstIt =  mSchedules.begin();
                auto nextTime = firstIt->first;
                mToRun = std::move(firstIt->second.second);
                auto tdiff = nextTime - tp;
                mScheduleIds.erase(firstIt->second.first);
                mSchedules.erase(firstIt);
                mSchedulesCv.wait_for(lg, tdiff, waitBreaker);
                continue;
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
    std::condition_variable mSchedulesCv;
    bool mCancelRequest;
    bool mAddRequest;
    std::mutex mSchedulesMutex;

    bool mRunning;
};
} // namespace bfc

#endif // __TIMER_HPP__
