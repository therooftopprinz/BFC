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

#include <bfc/function.hpp>

namespace bfc
{

template <typename function_t = light_function<void()>>
class thead_pool
{
public:
    using fn_t = function_t;
    thead_pool(size_t p_max_size = 4)
        : m_max_size(p_max_size)
    {
        std::unique_lock<std::mutex> lg(m_free_mtx);
        for (auto i=0u; i<m_max_size; i++)
        {
            m_free.emplace_back(i);
            m_pool.emplace_back(std::make_unique<thread_entry>());
            auto& entry = *(m_pool.back());
            entry.functor = {};

            entry.thread = std::thread([this, use_index = i, &entry]()
                {
                    auto pred = [&]() {return entry.functor || !m_is_running;};
                    while(true)
                    {
                        std::unique_lock<std::mutex> lg(entry.entry_mtx);

                        if (entry.functor)
                        {
                            entry.functor();
                            entry.functor = {};
                            std::unique_lock<std::mutex> lg(m_free_mtx);
                            m_free.emplace_back(use_index);
                        }

                        if (!m_is_running)
                        {
                            return;
                        }

                        m_max_size_cv.notify_one();

                        entry.thread_cv.wait(lg, pred);
                    }
                });
        }
    }
    
    ~thead_pool()
    {
        m_is_running = false;
        for (auto& i : m_pool)
        {
            i->thread_cv.notify_one();
            i->thread.join();
        }
    }
    void execute(function_t p_functor)
    {
        std::unique_lock<std::mutex> lg(m_free_mtx);

        m_max_size_cv.wait(lg, [this](){
                return m_free.size();
            });

        auto use_index = m_free.back();
        m_free.pop_back();
        auto& entry = *m_pool[use_index];

        std::unique_lock<std::mutex> lgEntry(entry.entry_mtx);
        entry.functor = std::move(p_functor);
        entry.thread_cv.notify_one();
    }

    size_t count_active() const
    {
        std::unique_lock<std::mutex> lg(m_free_mtx);
        return m_pool.size() - m_free.size();
    }

    size_t size() const
    {
        std::unique_lock<std::mutex> lg(m_free_mtx);
        return m_pool.size();
    }
private:
    struct thread_entry
    {
        function_t functor;
        std::thread thread;
        std::condition_variable thread_cv;
        std::mutex entry_mtx;
    };

    bool m_is_running = true;
    std::vector<std::unique_ptr<thread_entry>> m_pool;
    size_t m_max_size = 8;
    std::condition_variable m_max_size_cv;
    std::vector<size_t> m_free;
    mutable std::mutex m_free_mtx;
};

} // namespace bfc

#endif // __THREADPOOL_HPP__
