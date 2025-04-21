#ifndef __BFC_FUNCTION_HPP__
#define __BFC_FUNCTION_HPP__

#include <cstddef>
#include <cstring>
#include <functional>

namespace bfc
{

template <size_t N, typename return_t, typename... args_t>
class function
{
public:
    function() = default;

    function(const function &p_other)
    {
        if (p_other)
        {
            p_other.m_copier(m_object, p_other.m_object);
        }

        set(p_other);
    }

    function(function &&p_other)
    {
        if (p_other)
        {
            p_other.m_mover(m_object, p_other.m_object);
        }

        set(p_other);
        p_other.clear();
    }

    template <typename callable_t, std::enable_if_t<!std::is_same_v<std::remove_reference_t<callable_t>, function>> *p = nullptr>
    function(callable_t &&p_obj)
    {
        set(std::forward<callable_t>(p_obj));
    }

    function &operator=(const function &p_other)
    {
        reset();

        if (p_other)
        {
            p_other.m_copier(m_object, p_other.m_object);
        }

        set(p_other);
        return *this;
    }

    function &operator=(function &&p_other)
    {
        reset();

        if (p_other)
        {
            p_other.m_mover(m_object, p_other.m_object);
        }

        set(p_other);
        p_other.clear();
        return *this;
    }

    template <typename callable_t>
    function &operator=(callable_t &&p_obj)
    {
        reset();
        set(std::forward<callable_t>(p_obj));
        return *this;
    }

    ~function()
    {
        reset();
    }

    operator bool() const
    {
        return m_fn;
    }

    void reset()
    {
        if (m_fn)
        {
            m_destroyer(m_object);
        }

        clear();
    }

    template <typename... T>
    return_t operator()(T &&... pArgs) const
    {
        if (m_fn)
        {
            return m_fn(m_object, std::forward<T>(pArgs)...);
        }
        else
        {
            throw std::bad_function_call();
        }
    }

private:
    template <typename callable_t, std::enable_if_t<!std::is_same_v<std::remove_reference_t<callable_t>, function>> *p = nullptr>
    void set(callable_t &&p_obj)
    {
        using callable_tType = std::decay_t<callable_t>;
        static_assert(N >= sizeof(callable_tType));
        new (m_object) callable_tType(std::forward<callable_tType>(p_obj));
        m_destroyer = [](void *p_obj) {
            ((callable_tType *)p_obj)->~callable_tType();
        };

        m_copier = [](void *p_obj, const void *p_other) {
            new (p_obj) callable_tType(*(const callable_tType *)p_other);
        };

        m_mover = [](void *p_obj, void *p_other) {
            new (p_obj) callable_tType(std::move(*(callable_tType *)p_other));
        };

        m_fn = [](const void *p_obj, args_t... pArgs) -> return_t {
            return (*((callable_tType *)p_obj))(std::forward<args_t>(pArgs)...);
        };
    }

    void set(const function &p_other)
    {
        m_fn = p_other.m_fn;
        m_destroyer = p_other.m_destroyer;
        m_copier = p_other.m_copier;
        m_mover = p_other.m_mover;
    }

    void set(const std::nullptr_t)
    {
        clear();
    }

    void clear()
    {
        m_fn = nullptr;
    }

    uint8_t m_object[N];
    return_t (*m_fn)(const void *, args_t...) = nullptr;
    void (*m_destroyer)(void *) = nullptr;
    void (*m_copier)(void *, const void *) = nullptr;
    void (*m_mover)(void *, void *) = nullptr;
};

template <size_t N, typename T>
struct function_type_helper;
template <size_t N, typename return_t, typename... args_t>
struct function_type_helper<N, return_t(args_t...)>
{
    using type = function<N, return_t, args_t...>;
};

template <typename function_t>
using ulight_function = typename function_type_helper<8, function_t>::type;
template <typename function_t>
using light_function = typename function_type_helper<24, function_t>::type;
template <typename function_t>
using big_function = typename function_type_helper<32, function_t>::type;

} // namespace bfc

#endif // __BFC_FUNCTION_HPP__