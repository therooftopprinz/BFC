#ifndef __SINGLETON_HPP__
#define __SINGLETON_HPP__

#include <stdexcept>

namespace bfc
{

template<typename T>
class Singleton
{
public:
    template<typename... U>
    static T& instantiate(U&&... pArgs)
    {
        static T instance(std::forward<U>(pArgs)...);
        return get(&instance);
    }

    static T& get(T* pInstance = nullptr)
    {
        static T* instance(pInstance);
        if (nullptr == instance)
        {
            throw std::runtime_error("get called before instantiate");
        }
        return *instance;
    }
};

} // namespace bfc

#endif // __SINGLETON_HPP__