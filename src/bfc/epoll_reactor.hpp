#ifndef __BFC_EPOLL_REACTOR_HPP__
#define __BFC_EPOLL_REACTOR_HPP__

#include <mutex>

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/unistd.h>

#include <bfc/function.hpp>
#include <bfc/reactor.hpp>

namespace bfc
{
template <typename cb_t = light_function<void()>>
class epoll_reactor : public reactor<int, cb_t>
{
public:
    epoll_reactor(const epoll_reactor&) = delete;
    void operator=(const epoll_reactor&) = delete;

    epoll_reactor(size_t p_cache_size = 64)
        : m_event_cache(p_cache_size)
        , m_epoll_fd(epoll_create1(0))
    {
        if (-1 == m_epoll_fd)
        {
            throw std::runtime_error(strerror(errno));
        }

        m_event_d = eventfd(0, EFD_SEMAPHORE);

        if (-1 == m_event_d)
        {
            throw std::runtime_error(strerror(errno));
        }

        add(m_event_d, [this](){
                uint64_t one;
                read(m_event_d, &one, sizeof(one));
            });
    }

    ~epoll_reactor()
    {
        stop();
        close(m_event_d);
        close(m_epoll_fd);
    }

    bool add(int p_fd, cb_t p_read_cb) override
    {
        std::unique_lock<std::mutex> lg(m_cb_map_mtx);

        if (m_cb_map.count(p_fd))
        {
            return false;
        }

        epoll_event event{};
        event.data.fd = p_fd;
        event.events = EPOLLIN | EPOLLRDHUP;

        auto res = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, p_fd, &event);
        if (-1 == res)
        {
            return false;
        }

        m_cb_map.emplace(p_fd, std::move(p_read_cb));

        return true;
    }

    bool remove(int p_fd) override
    {
        std::unique_lock<std::mutex> lg(m_cb_map_mtx);

        epoll_event event;
        auto res = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, p_fd, &event);

        if (-1 == res)
        {
            return false;
        }

        if (m_cb_map.count(p_fd))
        {
            m_cb_map.erase(p_fd);
        }
        return true;
    }

    void run() override
    {
        m_running = true;
        while (m_running)
        {

            auto nfds = epoll_wait(m_epoll_fd, m_event_cache.data(), m_event_cache.size(), -1);
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
                std::unique_lock<std::mutex> lg(m_cb_map_mtx);
                auto it = m_cb_map.find(m_event_cache[i].data.fd);
                if (m_cb_map.end() != it)
                {
                    auto fn = it->second;
                    lg.unlock();
                    fn();
                }
            }
        }
    }

    void stop() override
    {
        m_running = false;
        wakeup();
    }

private:
    void wakeup() override
    {
        uint64_t one = 1;
        write(m_event_d, &one, sizeof(one));
    }

    std::vector<epoll_event> m_event_cache;
    std::unordered_map<int, cb_t> m_cb_map;
    std::mutex m_cb_map_mtx;

    int m_epoll_fd;
    int m_event_d;
    bool m_running;
};

} // namespace bfc

#endif // __BFC_EPOLL_REACTOR_HPP__
