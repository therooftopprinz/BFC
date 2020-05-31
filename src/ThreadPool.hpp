#ifndef __THREADPOOL_HPP__
#define __THREADPOOL_HPP__

#include <mutex>
#include <vector>
#include <thread>
#include <future>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <cstddef>

#include <bfc/FixedFunctionObject.hpp>

namespace bfc
{

template <typename FunctorType = LightFn<void()>>
class ThreadPool
{
public:
    using Functor = FunctorType;
    ThreadPool(size_t pMaxSize = 4)
        : mMaxSize(pMaxSize)
    {
        std::unique_lock<std::mutex> lgFreeList(mFreeListMutex);
        for (auto i=0u; i<mMaxSize; i++)
        {
            mFreeList.emplace_back(i);
            mPool.emplace_back(std::make_unique<ThreadEntry>());
            auto& entry = *(mPool.back());
            entry.functor = {};

            entry.thread = std::thread([this, useIndex = i, &entry]()
                {
                    auto pred = [&]() {return entry.functor || !mIsRunning;};
                    while(true)
                    {
                        std::unique_lock<std::mutex> lgEntry(entry.entryMutex);

                        if (entry.functor)
                        {
                            entry.functor();
                            entry.functor = {};
                            std::unique_lock<std::mutex> lgFreeList(mFreeListMutex);
                            mFreeList.emplace_back(useIndex);
                        }

                        if (!mIsRunning)
                        {
                            return;
                        }

                        mMaxSizeCv.notify_one();

                        entry.threadCv.wait(lgEntry, pred);
                    }
                });
        }
    }
    
    ~ThreadPool()
    {
        mIsRunning = false;
        for (auto& i : mPool)
        {
            i->threadCv.notify_one();
            i->thread.join();
        }
    }
    void execute(Functor pFunctor)
    {
        std::unique_lock<std::mutex> lgFreeList(mFreeListMutex);

        mMaxSizeCv.wait(lgFreeList, [this](){
                return mFreeList.size();
            });

        auto useIndex = mFreeList.back();
        mFreeList.pop_back();
        auto& entry = *mPool[useIndex];

        std::unique_lock<std::mutex> lgEntry(entry.entryMutex);
        entry.functor = std::move(pFunctor);
        entry.threadCv.notify_one();
    }

    std::size_t countActive() const
    {
        std::unique_lock<std::mutex> lgFreeList(mFreeListMutex);
        return mPool.size() - mFreeList.size();
    }

    std::size_t size() const
    {
        std::unique_lock<std::mutex> lgFreeList(mFreeListMutex);
        return mPool.size();
    }
private:
    struct ThreadEntry
    {
        Functor functor;
        std::thread thread;
        std::condition_variable threadCv;
        std::mutex entryMutex;
    };

    bool mIsRunning = true;
    std::vector<std::unique_ptr<ThreadEntry>> mPool;
    size_t mMaxSize = 8;
    std::condition_variable mMaxSizeCv;
    std::vector<std::size_t> mFreeList;
    mutable std::mutex mFreeListMutex;
};

} // namespace bfc

#endif // __THREADPOOL_HPP__
