
#ifndef __BFC_DEFAULT_REACTOR_HPP__
#define __BFC_DEFAULT_REACTOR_HPP__


#ifdef TARGET_LINUX
#include <bfc/epoll_reactor.hpp>
#endif

#ifdef TARGET_POSIX
#include <bfc/poll_reactor.hpp>
#endif

namespace bfc
{

#ifdef TARGET_LINUX
template <typename cb_t = light_function<void()>>
using default_reactor = epoll_reactor<cb_t>;
#endif

#ifdef TARGET_POSIX
template <typename cb_t = light_function<void()>>
using default_reactor = poll_reactor<cb_t>;
#endif


} // namespace bfc

#endif // __BFC_DEFAULT_REACTOR_HPP__
