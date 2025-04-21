#ifndef __BFC_EPOLL_REACTOR_HPP__
#define __BFC_EPOLL_REACTOR_HPP__

#include <mutex>
#include <atomic>

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/unistd.h>

#include <bfc/function.hpp>
#include <bfc/reactor.hpp>

namespace bfc
{

template <typename T, typename cb_t = light_function<void()>>
class cv_event_queue
{
public:
    using reactor_t = reactor<cv_event_queue<T, cb_t>;

    cv_event_queue(reactor_t* reactor = nullptr)
        : reactor (reactor)
    {}

    ~cv_event_queue()
    {}

    template <typename U>
    size_t push(U&& u)
    {
        size_t rv;
        {
            std::unique_lock<std::mutex> lg(queue_mutex);
            queue.emplace_back(std::forward<U>(u));
            rv = queue.size();
        }

        if (reactor)
        {
            reactor->wakeup();
        }

        return rv;
    }

    std::list<T> pop()
    {
        std::list<T> rv;

        std::unique_lock<std::mutex> lg(queue_mutex);
        rv.splice(rv.end(), queue, queue.begin(), queue.end());

        return rv;
    }

    size_t size()
    {
        std::unique_lock<std::mutex> lg(queue_mutex);
        return queue.size();
    }

protected:
    std::list<T> queue;
    std::mutex queue_mutex;
    reactor_t* reactor = nullptr;
};

template <typename T, typename cb_t = light_function<void()>>
class cv_reactor : public reactor<cv_event_queue<T>*, cb_t>
{
public:
    using fd_t = cv_event_queue<T>*;

    cv_reactor(const cv_reactor&) = delete;
    void operator=(const cv_reactor&) = delete;

    cv_reactor(uint64_t timeout=100)
        : m_timeout_ms(timeout)
    {}

    ~cv_reactor()
    {
        stop();
    }

    bool add(fd_t p_fd, reactor_cb_t p_read_cb) override
    {
        std::unique_lock lg(m_read_cb_mtx);
        if (m_read_cb_map.count(p_fd))
        {
            return false;
        }

        auto [it, added] = m_read_cb_map.emplace(p_fd, std::move(p_read_cb));
        return added;
    }

    bool remove(fd_t p_fd) override
    {
        std::unique_lock lg(m_read_cb_mtx);
        if (!m_read_cb_map.count(p_fd))
        {
            return false;
        }
        m_read_cb_map.erase(&reader);
        return true;
    }

    void run()
    {
        m_running = true;
        while (m_running)
        {
            std::unique_lock lg(m_wakeup_mtx);

            m_cv.wait_for(lg, std::chrono::milliseconds(m_timeout_ms), [this]()
                {
                    return m_wakeup_req.load();
                });

            lg.unlock();

            bool test = true; 
            if (m_wakeup_req.compare_exchange_strong(test, false))
            {
                std::unique_lock lg(m_read_cb_mtx);
                for (auto& i : m_read_cb_map)
                {
                    if (i.first->size())
                    {
                        i.second();
                    }
                }
            }
        }
    }

    void stop()
    {
        m_running = false;
        wakeup();
    }

 private:
    void wakeup() override
    {
        m_wakeup_req = true;
        m_cv.notify_one();
    }

    size_t m_timeout_ms = 100;

    std::map<int, reactor_cb_t> m_read_cb_map;
    std::mutex m_read_cb_mtx;

    std::mutex m_wakeup_mtx;
    std::atomic_bool m_wakeup_req = false;
    std::condition_variable m_cv;

    bool m_running;

};

} // namespace bfc

#endif // __BFC_EPOLL_REACTOR_HPP__
