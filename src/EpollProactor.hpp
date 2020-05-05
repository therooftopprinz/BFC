#ifndef __EPOLLPROACTOR_HPP__
#define __EPOLLPROACTOR_HPP__

#include <mutex>

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/unistd.h>

#include <bfc/FixedFunctionObject.hpp>
#include <bfc/ThreadPool.hpp>

namespace bfc
{

class EpollProactor
{
public:

    using Callback = LightFn<void(int)>;

    EpollProactor(const EpollProactor&) = delete;
    void operator=(const EpollProactor&) = delete;

    EpollProactor()
        : mEpollFd(epoll_create1(0))
    {
        if (-1 == mEpollFd)
        {
            throw std::runtime_error(strerror(errno));
        }

        mEventFd = eventfd(0, EFD_SEMAPHORE);

        if (-1 == mEventFd)
        {
            throw std::runtime_error(strerror(errno));
        }

        addHandler(mEventFd, [this](){
                uint64_t one;
                static_assert(8 == sizeof(one));
                read(mEventFd, &one, 8);
            });
    }

    ~EpollProactor()
    {
        stop();
        close(mEventFd);
        close(mEpollFd);
    }

    void addHandler(int pFd, Callback pReadCallback)
    {
        std::unique_lock<std::mutex> lg(mCallbackMapToAddMutex);
        mCallbackMapToAdd.emplace_back(pFd, std::move(pReadCallback));
        notifyEpoll();
    }

    void removeHandler(int pFd)
    {
        std::unique_lock<std::mutex> lg(mCallbackMapToRemoveMutex);
        mCallbackMapToRemove.emplace_back(pFd);
        notifyEpoll();
    }

    void run()
    {
        mRunning = true;
        while (mRunning)
        {
            auto nfds = epoll_wait(mEpollFd, mEventCache.data(), mEventCache.size(), -1);
            if (-1 == nfds)
            {
                if (EINTR == errno)
                {
                    throw std::runtime_error(strerror(errno));
                }
                continue;
            }

            for (int i=0; i<nfds; i++)
            {
                mTp.execute(mCallbackMap.at(mEventCache[i].data.fd));
            }

            processCallbackToAdd();
            processCallbackToRemove();
        }
    }

    void stop()
    {
        mRunning = false;
        notifyEpoll();
    }

private:

    void processCallbackToAdd()
    {
        std::unique_lock<std::mutex> lg(mCallbackMapToAddMutex);
        for (auto& i : mCallbackMapToAdd)
        {
            epoll_event event;
            event.data.fd = i.first;
            event.events = EPOLLIN;

            if (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, i.first, &event) == -1)
            {
                throw std::runtime_error(strerror(errno));
            }

            mCallbackMap.emplace(i.first, std::move(i.second));
            mEventCache.emplace_back();
        }

        mCallbackMapToAdd.clear();
    }

    void processCallbackToRemove()
    {
        std::unique_lock<std::mutex> lg(mCallbackMapToRemoveMutex);
        for (auto& i : mCallbackMapToRemove)
        {
            epoll_event event;
            if (epoll_ctl(mEpollFd, EPOLL_CTL_DEL, i, &event) == -1)
            {
                throw std::runtime_error(strerror(errno));
            }

            mCallbackMap.erase(i);
            mEventCache.pop_back();
        }

        mCallbackMapToRemove.clear();
    }

    void notifyEpoll()
    {
        uint64_t one = 1;
        static_assert(8 == sizeof(one));
        write(mEventFd, &one, 8);
    }

    std::vector<epoll_event> mEventCache;
    std::unordered_map<int, Callback> mCallbackMap;

    std::vector<std::pair<int, Callback>> mCallbackMapToAdd;
    std::vector<int> mCallbackMapToRemove;

    std::mutex mCallbackMapToAddMutex;
    std::mutex mCallbackMapToRemoveMutex;

    ThreadPool<> mTp;
    int mEpollFd;
    int mEventFd;
    bool mRunning;
};

} // namespace bfc

#endif // __EPOLLPROACTOR_HPP__
