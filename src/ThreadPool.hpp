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

template <typename FunctorType = LightFunctionObject<void()>>
class ThreadPool
{
public:
    using Functor = FunctorType;
    ~ThreadPool()
    {
        mIsRunning = false;
        for (auto& i : mPool)
        {
            i->threadCv.notify_one();
            i->thread.join();
        }
    }
    void execute(const Functor& pFunctor)
    {
        std::size_t useIndex; 

        std::unique_lock<std::mutex> lg(mFreeListMutex);
        if (mFreeList.size())
        {
            useIndex = mFreeList.back();
            mFreeList.pop_back();
        }
        else
        {
            mPool.emplace_back(std::make_unique<ThreadEntry>());
            auto& entry = *(mPool.back());
            useIndex = mPool.size()-1;
            mPool[useIndex]->thread = std::thread([this, useIndex, &entry]()
            {
                auto pred = [&]() {return entry.functor || !mIsRunning;};
                while(true)
                {
                    std::unique_lock<std::mutex> lg(entry.entryMutex);
                    entry.threadCv.wait(lg, pred);
                    if (!mIsRunning)
                    {
                        return;
                    }
                    if (entry.functor)
                    {
                        (*(entry.functor))();
                        entry.functor = nullptr;
                        std::unique_lock<std::mutex> lg(mFreeListMutex);
                        mFreeList.emplace_back(useIndex);
                    }
                }
            });
        }

        auto& threadEntry = *mPool[useIndex];
        threadEntry.functor = &pFunctor;
        threadEntry.threadCv.notify_one();
    }

    std::size_t countActive() const
    {
        std::unique_lock<std::mutex> lg(mFreeListMutex);
        return mPool.size() - mFreeList.size();
    }

    std::size_t size() const
    {
        return mPool.size();
    }
private:
    struct ThreadEntry
    {
        const Functor* functor;
        std::thread thread;
        std::condition_variable threadCv;
        std::mutex entryMutex;
    };

    bool mIsRunning = true;
    std::vector<std::unique_ptr<ThreadEntry>> mPool;
    std::vector<std::size_t> mFreeList;
    mutable std::mutex mFreeListMutex;
};

} // namespace bfc

#endif // __THREADPOOL_HPP__
