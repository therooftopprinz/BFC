#ifndef __BFC_EPOLL_REACTOR_HPP__
#define __BFC_EPOLL_REACTOR_HPP__

#include <mutex>
#include <thread>
#include <deque>

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/unistd.h>

#include <bfc/function.hpp>

namespace bfc
{

namespace detail
{

template <typename cb_t = light_function<void()>>
struct epoll_reactor
{
    struct fd_ctx_t
    {
        int fd;
        epoll_event event;
        cb_t cb;
    };

    epoll_reactor(const epoll_reactor&) = delete;
    void operator=(const epoll_reactor&) = delete;

    epoll_reactor(size_t p_cache_size = 64)
        : m_event_cache(p_cache_size)
        , m_epoll_fd(epoll_create1(0))
    {
        m_event_fd = eventfd(0, EFD_SEMAPHORE);

        if (-1 == m_epoll_fd)
        {
            throw std::runtime_error(strerror(errno));
        }

        add(m_event_fd, EPOLLIN, [this](){
                uint64_t one;
                read(m_event_fd, &one, sizeof(one));
            });
    }

    ~epoll_reactor()
    {
        stop();
        close(m_event_fd);
        close(m_epoll_fd);
    }

    bool add(int p_fd, uint32_t event, cb_t cb)
    {
        std::unique_lock<std::mutex> lg(m_fd_map_mtx);
        if (m_fd_map.count(p_fd))
        {
            return false;
        }

        auto ctx = new fd_ctx_t();
        ctx->cb = std::move(cb);
        ctx->event.events = event;
        ctx->event.data.ptr = ctx;

        auto res = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, p_fd, &(ctx->event));
        if (-1 == res)
        {
            return false;
        }

        m_fd_map.emplace(p_fd, ctx);
        return true;
    }

    bool del(int p_fd)
    {
        std::unique_lock<std::mutex> lg(m_to_del_queue_mtx);
        m_to_del_queue.emplace_back(p_fd);
        wakeup();
        return true;
    }

    bool mod(int p_fd, uint32_t event)
    {
        std::unique_lock<std::mutex> lg(m_fd_map_mtx);
        auto it = m_fd_map.find(p_fd);
        if (m_fd_map.end() == it)
        {
            return false;
        }
        auto ctx = it->second;
        lg.unlock();

        ctx->event.events = event;

        auto res = epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, p_fd, &(ctx->event));

        if (-1 == res)
        {
            return false;
        }

        return true;
    }

    void run()
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
                auto* ctx = (fd_ctx_t*) m_event_cache[i].data.ptr;
                if (ctx->cb)
                {
                    ctx->cb();
                }
            }

            std::unique_lock lg(m_to_del_queue_mtx);
            for (auto i : m_to_del_queue)
            {
                do_del(i);
            }
            m_to_del_queue.clear();
        }
    }

    void stop()
    {
        m_running = false;
        wakeup();
    }

private:
    bool do_del(int p_fd)
    {
        std::unique_lock<std::mutex> lg(m_fd_map_mtx);

        auto it = m_fd_map.find(p_fd);

        if (m_fd_map.end() == it)
        {
            return false;
        }

        auto res = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, p_fd, nullptr);
        if (-1 == res)
        {
            return false;
        }

        delete it->second;

        m_fd_map.erase(p_fd);

        return true;
    }


    void wakeup()
    {
        uint64_t one = 1;
        write(m_event_fd, &one, sizeof(one));
    }

    std::vector<epoll_event> m_event_cache;

    std::mutex m_fd_map_mtx;
    std::unordered_map<int, fd_ctx_t*> m_fd_map;

    std::mutex m_to_del_queue_mtx;
    std::deque<int> m_to_del_queue;

    int m_epoll_fd;
    int m_event_fd;
    bool m_running;
};

} // namespace detail

template <typename cb_t = light_function<void()>>
class epoll_reactor
{

public:
    epoll_reactor(const epoll_reactor&) = delete;
    void operator=(const epoll_reactor&) = delete;

    epoll_reactor(){}
    ~epoll_reactor(){}

    bool add_read_rdy(int p_fd, cb_t cb)
    {
        return m_reader.add(p_fd, EPOLLIN|EPOLLRDHUP, std::move(cb));
    }

    bool rem_read_rdy(int p_fd)
    {
        return m_reader.del(p_fd);
    }

    bool req_read(int p_fd)
    {
        return true;
    }

    bool add_write_rdy(int p_fd, cb_t cb)
    {
        return m_writer.add(p_fd, 0, std::move(cb));
    }

    bool rem_write_rdy(int p_fd)
    {
        return m_writer.del(p_fd);
    }

    bool req_write(int p_fd)
    {
        return m_writer.mod(p_fd, EPOLLOUT|EPOLLONESHOT);
    }

    void run()
    {
        std::thread writer = std::thread([this](){m_writer.run();});
        m_reader.run();
        writer.join();
    }

    void stop()
    {
        m_writer.stop();
        m_reader.stop();
    }

private:
    detail::epoll_reactor<cb_t> m_reader;
    detail::epoll_reactor<cb_t> m_writer;
};

} // namespace bfc

#endif // __BFC_EPOLL_REACTOR_HPP__
