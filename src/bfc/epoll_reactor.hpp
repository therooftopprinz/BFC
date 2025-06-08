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
    struct fd_ctx_s
    {
        int fd = -1;
        epoll_event event;
        cb_t cb = nullptr;
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

        m_event_fd_ctx.fd = m_event_fd;
        m_event_fd_ctx.event.events = EPOLLIN;
        m_event_fd_ctx.cb = [this](){
                uint64_t one;
                auto res [[maybe_unused]] = read(m_event_fd, &one, sizeof(one));
            };

        add(m_event_fd_ctx);
    }

    ~epoll_reactor()
    {
        stop();
        close(m_event_fd);
        close(m_epoll_fd);
    }

    int add(fd_ctx_s& ctx)
    {
        ctx.event.data.ptr = &ctx;
        return epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, ctx.fd, &(ctx.event));
    }

    int del(fd_ctx_s& ctx)
    {
        return epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, ctx.fd, nullptr);
    }

    int mod(fd_ctx_s& ctx)
    {
        return epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, ctx.fd, &(ctx.event));
    }

    void run(cb_t cb = nullptr)
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
                auto* ctx = (fd_ctx_s*) m_event_cache[i].data.ptr;
                if (ctx->cb)
                {
                    ctx->cb();
                }
            }

            {
                std::unique_lock lg(m_wake_up_cb_mtx);
                for (auto& cb : m_wake_up_cb)
                {
                    cb();
                }

                m_wake_up_cb.clear();
            }

            if (cb)
            {
                cb();
            }
        }
    }

    void stop()
    {
        m_running = false;
        wake_up();
    }

    void wake_up(cb_t cb = nullptr)
    {
        if (cb)
        {
            std::unique_lock lg(m_wake_up_cb_mtx);
            m_wake_up_cb.emplace_back(std::move(cb));
        }

        uint64_t one = 1;
        auto res [[maybe_unused]] = write(m_event_fd, &one, sizeof(one));
    }

private:
    std::vector<epoll_event> m_event_cache;

    std::mutex m_wake_up_cb_mtx;
    std::vector<cb_t> m_wake_up_cb;

    int m_epoll_fd;
    int m_event_fd;
    bool m_running;

    fd_ctx_s m_event_fd_ctx;
};

} // namespace detail

template <typename cb_t = light_function<void()>>
class epoll_reactor
{
    using reactor_t = detail::epoll_reactor<cb_t>;

public:
    using fd_t = int;

    class context
    {
    public:
        context() = default;
        context(const context&) = delete;

        context(context&& other)
        {
            move_from(std::move(other));
        }

        context(fd_t fd)
        {
            reader.fd = fd;
            writer.fd = dup(fd);
        }

        ~context()
        {
            if (-1 != writer.fd)
            {
                close(writer.fd);
            }
        }

        context& operator=(context&& other)
        {
            move_from(std::move(other));
            return *this;
        }

    private:
        void move_from(context&& other)
        {
            reader.fd = other.reader.fd;
            reader.event = other.reader.event;
            reader.cb = std::move(other.reader.cb);
            other.reader.fd = -1;

            writer.fd = other.writer.fd;
            writer.event = other.writer.event;
            writer.cb = std::move(other.writer.cb);
            other.writer.fd = -1;
        }

        friend class epoll_reactor;

        typename reactor_t::fd_ctx_s reader;
        typename reactor_t::fd_ctx_s writer;
    };

    epoll_reactor(const epoll_reactor&) = delete;
    void operator=(const epoll_reactor&) = delete;

    epoll_reactor(){}
    ~epoll_reactor(){}

    context make_context(fd_t fd)
    {
        return context(fd);
    }

    int get_last_error_code()
    {
        return errno;
    }

    std::string get_last_error()
    {
        return strerror(errno);
    }

    bool add_read_rdy(context& ctx, cb_t cb)
    {
        ctx.reader.cb = std::move(cb);
        ctx.reader.event.events = EPOLLIN|EPOLLRDHUP;
        return m_reactor.add(ctx.reader) == 0;
    }

    bool rem_read_rdy(context& ctx)
    {
        return m_reactor.del(ctx.reader) == 0;
    }

    bool req_read(context&)
    {
        return true;
    }

    bool add_write_rdy(context& ctx, cb_t cb)
    {
        ctx.writer.cb = std::move(cb);
        ctx.writer.event.events = 0;
        return m_reactor.add(ctx.writer) == 0;
    }

    bool rem_write_rdy(context& ctx)
    {
        return m_reactor.del(ctx.writer) == 0;
    }

    bool req_write(context& ctx)
    {
        ctx.writer.event.events = EPOLLOUT|EPOLLONESHOT;
        return m_reactor.mod(ctx.writer) == 0;
    }

    void wake_up(cb_t cb)
    {
        m_reactor.wake_up(std::move(cb));
    }

    void run(cb_t cb = nullptr)
    {
        m_reactor.run(std::move(cb));
    }

    void stop()
    {
        m_reactor.stop();
    }

private:
    reactor_t m_reactor;
};

} // namespace bfc

#endif // __BFC_EPOLL_REACTOR_HPP__
