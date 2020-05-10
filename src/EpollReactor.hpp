#ifndef __EpollReactor_HPP__
#define __EpollReactor_HPP__

#include <mutex>

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/unistd.h>

#include <bfc/FixedFunctionObject.hpp>

namespace bfc
{

class EpollReactor
{
public:

    using Callback = LightFn<void()>;
    using StatusCallback = LightFn<void(int, int)>;

    EpollReactor(const EpollReactor&) = delete;
    void operator=(const EpollReactor&) = delete;

    EpollReactor()
        : mEventCache(mEventCacheSize)
        , mEpollFd(epoll_create1(0))
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

    ~EpollReactor()
    {
        stop();
        close(mEventFd);
        close(mEpollFd);
    }

    bool addHandler(int pFd, Callback pReadCallback)
    {
        std::unique_lock<std::mutex> lgCallback(mCallbackMapMutex);

        if (mCallbackMap.count(pFd))
        {
            return false;
        }

        epoll_event event{};
        event.data.fd = pFd;
        event.events = EPOLLIN | EPOLLRDHUP;

        auto res = epoll_ctl(mEpollFd, EPOLL_CTL_ADD, pFd, &event);
        if (-1 == res)
        {
            return false;
        }

        mCallbackMap.emplace(pFd, std::move(pReadCallback));

        return true;
    }

    bool removeHandler(int pFd)
    {
        std::unique_lock<std::mutex> lgCallback(mCallbackMapMutex);

        epoll_event event;
        auto res = epoll_ctl(mEpollFd, EPOLL_CTL_DEL, pFd, &event);

        if (-1 == res)
        {
            return false;
        }

        if (mCallbackMap.count(pFd))
        {
            mCallbackMap.erase(pFd);
        }
        return true;
    }

    void run()
    {
        mRunning = true;
        while (mRunning)
        {

            auto nfds = epoll_wait(mEpollFd, mEventCache.data(), mEventCache.size(), -1);
            if (-1 == nfds)
            {
                if (EINTR != errno)
                {
                    throw std::runtime_error(strerror(errno));
                }
                continue;
            }

            for (int i=0; i<nfds; i++)
            {
                std::unique_lock<std::mutex> lg(mCallbackMapMutex);
                auto foundIt = mCallbackMap.find(mEventCache[i].data.fd);
                if (mCallbackMap.end() != foundIt)
                {
                    auto fn = foundIt->second;
                    lg.unlock();
                    fn();
                }
            }
        }
    }

    void stop()
    {
        mRunning = false;
        notifyEpoll();
    }

private:
    void notifyEpoll()
    {
        uint64_t one = 1;
        static_assert(8 == sizeof(one));
        write(mEventFd, &one, 8);
    }

    size_t mEventCacheSize = 64;
    std::vector<epoll_event> mEventCache;
    std::unordered_map<int, Callback> mCallbackMap;
    std::mutex mCallbackMapMutex;

    int mEpollFd;
    int mEventFd;
    bool mRunning;
};

} // namespace bfc

#endif // __EpollReactor_HPP__
