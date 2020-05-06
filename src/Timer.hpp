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
        std::unique_lock<std::mutex> lg(mSchedulesMutex);
        mAddRequest = true;
        mToAddList.emplace_back(pDiff, ScheduleIdFn(mIdCtr, std::move(pCb)));
        mSchedulesCv.notify_one();
        return mIdCtr++;
    }

    void cancel(int pId)
    {
        std::unique_lock<std::mutex> lg(mToCancelListMutex);
        mCancelRequest = true;
        mToCancelList.emplace_back(pId);
        mSchedulesCv.notify_one();
    }

    void run()
    {
        mRunning = true;
        auto waitBreaker = [this](){return mAddRequest|| !mRunning || mCancelRequest;};
        while (mRunning)
        {
            std::unique_lock<std::mutex> lg(mSchedulesMutex);
            auto tp = std::chrono::high_resolution_clock::now();

            {
                std::unique_lock<std::mutex> lg(mToCancelListMutex);

                for (auto i : mToCancelList)
                {
                    if (!mScheduleIds.count(i))
                    {
                        continue;
                    }

                    auto it = mScheduleIds.at(i);
                    mSchedules.erase(it);
                    mScheduleIds.erase(i);
                }

                mCancelRequest = false;
                mToCancelList.clear();
            }
            {
                std::unique_lock<std::mutex> lg(mToAddListMutex);
                for (auto& i : mToAddList)
                {
                    auto tpdiff = tp + i.first;
                    auto id = i.second.first;
                    auto res = mSchedules.emplace(tpdiff, std::move(i.second));
                    mScheduleIds.emplace(id, res);
                }

                mAddRequest = false;
                mToAddList.clear();
            }

            auto res = mSchedules.equal_range(tp);

            for (auto i = mSchedules.begin(); i != mSchedules.end() && i != res.second; i++)
            {
                i->second.second();
                mScheduleIds.erase(i->second.first);
            }

            mSchedules.erase(mSchedules.begin(), res.second);

            if (mSchedules.size())
            {
                auto next = mSchedules.begin()->first;
                auto tdiff = next - tp;
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
    int  mIdCtr=0;

    std::vector<int> mToCancelList;
    std::mutex mToCancelListMutex;

    std::vector<std::pair<std::chrono::nanoseconds, ScheduleIdFn>> mToAddList;
    std::mutex mToAddListMutex;

    std::condition_variable mSchedulesCv;
    std::mutex mSchedulesMutex;
    bfc::ThreadPool<> mTp;
    bool mCancelRequest;
    bool mAddRequest;
    bool mRunning;
};
} // namespace bfc

#endif // __TIMER_HPP__
