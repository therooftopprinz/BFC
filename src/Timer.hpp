#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include <map>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include <bfc/FixedFunctionObject.hpp>

namespace bfc
{

template <typename FunctorType = LightFn<void()>>
class Timer
{
public:
    using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
    Timer()
        : mNextSchedule(std::chrono::high_resolution_clock::now())
    {

    }
    int schedule(std::chrono::nanoseconds pDiff, FunctorType pCb)
    {
        std::unique_lock<std::mutex> lg(mSchedulesMutex);

        auto tp = std::chrono::high_resolution_clock::now();
        tp += pDiff;

        auto res = mSchedules.emplace(tp, std::move(pCb));
        mScheduleIds.emplace(mIdCtr, res);

        if (tp > mNextSchedule)
        {
            mNextSchedule = tp;
            mSchedulesCv.notify_one();
        }
        return mIdCtr++;
    }

    void cancel(int pId)
    {
        std::unique_lock<std::mutex> lg(mSchedulesMutex);

        auto it = mScheduleIds.at(pId);
        mSchedules.erase(it);
        mScheduleIds.erase(pId);

        mCancelRequest = true;

        lg.unlock();
        mSchedulesCv.notify_one();
    }

    void run()
    {
        mRunning = true;
        while (mRunning)
        {
            std::unique_lock<std::mutex> lg(mSchedulesMutex);
            auto tp = std::chrono::high_resolution_clock::now();
            mCancelRequest = false;
            auto res = mSchedules.equal_range(tp);

            for (auto i = mSchedules.begin(); i != res.second; i++)
            {
                lg.unlock();
                i->second();
                lg.lock();
            }

            mSchedules.erase(mSchedules.begin(), res.second);

            if (mSchedules.size())
            {
                mNextSchedule = mSchedules.begin()->first;
                auto tdiff = mNextSchedule - tp;
                mSchedulesCv.wait_for(lg, tdiff, [this](){
                    auto tp = std::chrono::high_resolution_clock::now();
                    return (tp < mNextSchedule) || !mRunning || mCancelRequest;
                });
                continue;
            }

            mSchedulesCv.wait(lg, [this](){
                return mSchedules.size() || !mRunning || mCancelRequest;
            });
        }
    }

    void stop()
    {
        mRunning = false;
        mSchedulesCv.notify_one();
    }

private:
    using ScheduleMap = std::multimap<TimePoint, FunctorType>;
    ScheduleMap mSchedules;
    std::unordered_map<int, typename ScheduleMap::iterator> mScheduleIds;
    int  mIdCtr=0;

    std::condition_variable mSchedulesCv;
    std::mutex mSchedulesMutex;
    TimePoint mNextSchedule;
    bool mCancelRequest;
    bool mRunning;
};
} // namespace bfc

#endif // __BUFFER_HPP__
