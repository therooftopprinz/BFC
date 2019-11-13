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

class ThreadPool
{
public:
    using Functor = LightFunctionObject<void()>;
    ~ThreadPool()
    {
        for (auto& i : mPool)
        {
            i->isRunning = false;
            i->threadCv.notify_one();
            i->thread.join();
        }
    }
    void execute(const Functor& pFunctor)
    {
        std::unique_lock<std::mutex> lg(mPoolMutex);
        std::size_t useIndex; 

        if (mFreeList.size())
        {
            useIndex = mFreeList.back();
            mFreeList.pop_back();
        }
        else
        {
            mPool.emplace_back(std::make_unique<ThreadEntry>());
            useIndex = mPool.size()-1;
            mPool[useIndex]->thread = std::thread([this, useIndex]() {
                run(useIndex);
            });
        }

        auto& threadEntry = *mPool[useIndex];
        threadEntry.functor = &pFunctor;
        threadEntry.threadCv.notify_one();
    } 
private:
    struct ThreadEntry
    {
        const Functor* functor;
        std::thread thread;
        std::condition_variable threadCv;
        bool isRunning = true;
        std::mutex entryMutex;
    };

    void run(std::size_t pUseIndex)
    {
        auto& entry = *mPool[pUseIndex]; 
        auto pred = [&]() {return entry.functor || !entry.isRunning;};
        while(true)
        {
            std::unique_lock<std::mutex> lg(entry.entryMutex);
            entry.threadCv.wait(lg, pred);
            if (!entry.isRunning)
            {
                return;
            }
            if (entry.functor)
            {
                (*(entry.functor))();
                entry.functor = nullptr;
                std::unique_lock<std::mutex> lg(mPoolMutex);
                mFreeList.emplace_back(pUseIndex);
            }
        }
    }

    std::vector<std::unique_ptr<ThreadEntry>> mPool;
    std::vector<std::size_t> mFreeList;
    std::mutex mPoolMutex;
};

} // namespace bfc

#endif // __THREADPOOL_HPP__
