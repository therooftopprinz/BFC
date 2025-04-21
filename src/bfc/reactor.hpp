#ifndef __BFC_REACTOR_HPP__
#define __BFC_REACTOR_HPP__

#include <mutex>

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/unistd.h>

#include <bfc/function.hpp>

namespace bfc
{

template <typename D = int, typename cb_t = light_function<void()>>
class reactor
{
public:
    virtual ~reactor() = default;
    virtual bool add(D p_fd, cb_t p_read_cb) = 0;
    virtual bool remove(D p_fd) = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
    virtual void wakeup() = 0;
};

} // namespace bfc

#endif // __BFC_REACTOR_HPP__
