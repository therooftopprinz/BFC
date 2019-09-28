#ifndef __LIGHTFN_HPP__
#define __LIGHTFN_HPP__

#include <cstddef>
#include <functional>

namespace bfc
{

template <std::size_t N, typename ReturnType, typename... Args>
class FixedFunctionObject
{
public:
FixedFunctionObject() = default;

template <typename CallableObj>
FixedFunctionObject(CallableObj&& pObj)
{
    set(std::forward<CallableObj>(pObj));
}

template <typename CallableObj>
FixedFunctionObject& operator=(CallableObj&& pObj)
{
    reset();
    set(std::forward<CallableObj>(pObj));
    return *this;
}

~FixedFunctionObject()
{
    reset();
}

void reset()
{
    if (mDestroyer)
    {
        mDestroyer(mObject);
    }
}
ReturnType operator()(Args&&... pArgs)
{
    if (mFn)
    {
        return mFn(mObject, std::forward<Args>(pArgs)...);
    }
    else
    {
        throw std::bad_function_call();
    }
}

private:
    template <typename CallableObj>
    void set(CallableObj&& pObj)
    {
        using CallableObjType = std::decay_t<CallableObj>;
        static_assert(N>=sizeof(CallableObjType));
        new (mObject) CallableObjType(std::forward<CallableObjType>(pObj));
        mDestroyer = [](void* pObj)
        {
            ((CallableObjType*)pObj)->~CallableObjType();
        };

        mFn = [](void* pObj, Args&&... pArgs) -> ReturnType
        {
            return (*((CallableObjType*)pObj))(std::forward<Args>(pArgs)...);
        };
    }
    uint8_t mObject[64];
    ReturnType (*mFn)(void*, Args&&...) = nullptr;
    void (*mDestroyer)(void*) = nullptr;
};

template <std::size_t N, typename T> struct FixedFunctionObjectTypeHelper;
template <std::size_t N, typename ReturnType, typename... ArgsType> struct FixedFunctionObjectTypeHelper<N, ReturnType(ArgsType...)>
{
    using type = FixedFunctionObject<N, ReturnType, ArgsType...>;
};

template <typename FnType>
using LightFunctionObject = typename FixedFunctionObjectTypeHelper<32, FnType>::type;

} // namespace bfc

#endif // __LIGHTFN_HPP__