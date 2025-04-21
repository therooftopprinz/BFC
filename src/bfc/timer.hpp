#ifndef __BFC_TIMER_HPP__
#define __BFC_TIMER_HPP__

#include <map>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <iostream>

#include <bfc/function.hpp>

namespace bfc
{

template <typename cb_t = std::function<void()>>
class timer
{
public:
    using timer_id_t = std::pair<int64_t, uint64_t>;

    timer_id_t wait_ms(int64_t for_ms, cb_t cb,
        int64_t now_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();)
    {
        std::unique_lock lg(m_cb_map_mtx);
        auto next_ms = now_ms + for_ms;
        auto timer_id = m_timer_ctr++;
        timer_id_t rv{next_ms, timer_id};
        m_cb_map.emplace(rv, cb);
        return rv;
    }

    bool cancel(timer_id_t id)
    {
        std::unique_lock lg(m_cb_map_mtx);
        return m_cb_map.erase(id);
    }

    void schedule(int64_t now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count())
    {
        std::list<decltype(m_cb_map)::node_type> extracted;
        {
            std::unique_lock lg(m_cb_map_mtx);
            auto it = m_cb_map.begin();
            while (it != m_cb_map.end())
            {
                auto next = it;
                next++;
                auto& timer = *it;
                if (now_ms >= timer.first.first)
                {
                    extracted.emplace_back(m_cb_map.extract(it));
                    it = next;
                    continue;
                }
                break;
            }
        }

        for (auto& cb : extracted)
        {
            cb.mapped()();
        }
    }

private:
    uint64_t m_timer_ctr = 0;
    std::map<timer_id_t, cb_t> m_cb_map;
    std::mutex m_cb_map_mtx;
};


} // namespace bfc

#endif // __BFC_TIMER_HPP__
