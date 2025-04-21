#ifndef __BFC_BUFFER_HPP__
#define __BFC_BUFFER_HPP__

#include <type_traits>

#include <bfc/function.hpp>

namespace bfc
{

template<typename T, typename Deleter = light_function<void(void*)>>
class simple_buffer
{
    static_assert(sizeof(T)==1);
public:
    template <typename U>
    simple_buffer(U* pData, size_t pSize, Deleter p_deleter = [](void* p_ptr){delete[] (T*)p_ptr;})
        : m_size(pSize)
        , m_data(pData)
        , m_deleter(std::move(p_deleter))
    {
        static_assert(sizeof(U)==1);
    }

    ~simple_buffer()
    {
        reset();
    }

    simple_buffer() = default;
    simple_buffer(const simple_buffer&) = delete;
    void operator=(const simple_buffer&) = delete;

    simple_buffer(simple_buffer&& p_other) noexcept
    {
        reset();
        transferOwnership(std::move(p_other));
    }

    simple_buffer& operator=(simple_buffer&& p_other) noexcept
    {
        reset();
        transfer(std::move(p_other));
        return *this;
    }

    T* data() const
    {
        return m_data;
    }

    size_t size() const
    {
        return m_size;
    }

    void reset() noexcept
    {
        if (m_data)
        {
            m_deleter((T*)m_data);
        }
        clear(std::move(*this));
    }

private:
    static void clear(simple_buffer&& p_other) noexcept
    {
        p_other.m_data = nullptr;
        p_other.m_size = 0;
        p_other.m_deleter = nullptr;
    }

    void transfer(simple_buffer&& p_other) noexcept
    {
        m_data = p_other.m_data;
        m_size = p_other.m_size;
        m_deleter = std::move(p_other.m_deleter);
        clear(std::move(p_other));
    };

    size_t m_size = 0;
    T* m_data = nullptr;
    Deleter  m_deleter;
};

using Buffer = simple_buffer<std::byte>;
using ConstBuffer = simple_buffer<const std::byte>;

template<typename T>
class simple_buffer_view
{
    using NCT = typename std::remove_const<T>::type;
    using CT = typename std::add_const<NCT>::type;
    static constexpr bool IS_T_CONST = std::is_const<T>::value;
    static_assert(sizeof(T)==1);
public:
    /* Constructors
    ** NCT -> CT  OK
    ** NCT -> NCT OK
    ** CT  -> CT  OK
    ** CT  -> NCT NOK
    */ 
    template<template<class> class U, class V>
    simple_buffer_view(const U<V>& pBuffer,
        typename std::enable_if<std::is_same<V,NCT>::value||std::is_same<V,T>::value>::type* = 0)
        : m_size(pBuffer.size())
        , m_data(pBuffer.data())
    {}

    template <typename U>
    simple_buffer_view(U* data, size_t size,
        typename std::enable_if<(std::is_const<U>::value && IS_T_CONST) || !std::is_const<U>::value>::type* = 0)
        : m_size(size)
        , m_data(data)
    {
        static_assert(sizeof(U)==1);
    }

    template<template<class> class U, class V>
    typename std::enable_if<std::is_same<V,NCT>::value||std::is_same<V,T>::value,simple_buffer_view>::type&
        operator=(U<V> pBuffer)
    {
        m_size = pBuffer.size();
        m_data = pBuffer.data();
        return *this;
    }

    T* data() const
    {
        return m_data;
    }

    size_t size() const
    {
        return m_size;
    }

private:
    size_t m_size = 0;
    T* m_data = nullptr;
};

using buffer = simple_buffer<std::byte>;
using buffer_view = simple_buffer_view<std::byte>;
using const_buffer_view = simple_buffer_view<const std::byte>;

} // namespace bfc

#endif // __BFC_BUFFER_HPP__
