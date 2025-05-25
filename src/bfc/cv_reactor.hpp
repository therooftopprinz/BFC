#ifndef __BFC_CV_REACTOR_HPP__
#define __BFC_CV_REACTOR_HPP__

#include <condition_variable>
#include <atomic>
#include <mutex>
#include <list>

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/unistd.h>

#include <bfc/function.hpp>
#include <bfc/event_queue.hpp>

namespace bfc
{

template <typename T, typename cb_t = light_function<void()>>
class cv_reactor
{
public:
    using context = reactive_event_queue<T, cv_reactor<T, cb_t>, cb_t>;

    cv_reactor(const cv_reactor&) = delete;
    void operator=(const cv_reactor&) = delete;

    cv_reactor(uint64_t timeout=100)
        : m_timeout_ms(timeout)
    {}

    ~cv_reactor()
    {
        stop();
    }

    context& make_context(context& ctx)
    {
        return ctx;
    }

    bool add_read_rdy(context& ctx, cb_t cb)
    {
        std::unique_lock lg(ctx.cb_mtx);
        ctx.cb = std::move(cb);
        return true;
    }

    bool remove_read_rdy(context& ctx)
    {
        std::unique_lock lg(ctx.cb_mtx);
        ctx.cb = nullptr;
        return true;
    }

    void run(cb_t cb = nullptr)
    {
        m_running = true;
        while (m_running)
        {
            {
                std::unique_lock lg(m_wakeup_mtx);

                m_cv.wait_for(lg, std::chrono::milliseconds(m_timeout_ms), [this]()
                    {
                        return m_wakeup_req;
                    });

                if (m_wakeup_req)
                {
                    m_wakeup_req = false;
                    auto cb_list = std::move(m_wakeup_cb_list);
                    lg.unlock();
                    for (auto& cb : cb_list)
                    {
                        cb();
                    }
                }
            }

            if (cb)
            {
                cb();
            }
        }
    }

    void wake_up(cb_t cb = nullptr)
    {
        std::unique_lock lg(m_wakeup_mtx);
        if (cb)
        {
            m_wakeup_cb_list.emplace_back(std::move(cb));
        }
        m_wakeup_req = true;
        m_cv.notify_one();
    }

    void stop()
    {
        m_running = false;
        wake_up();
    }

 private:
    size_t m_timeout_ms = 100;

    std::mutex m_wakeup_mtx;
    bool m_wakeup_req = false;
    std::list<cb_t> m_wakeup_cb_list;
    std::condition_variable m_cv;

    bool m_running;
};

} // namespace bfc

#endif // __BFC_EPOLL_REACTOR_HPP__
