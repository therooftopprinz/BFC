#ifndef __FIXEDFUNCTIONOBJECT_HPP__
#define __FIXEDFUNCTIONOBJECT_HPP__

#include <cstddef>
#include <cstring>
#include <functional>

#include <iostream>
template <typename>
void ptype()
{
    std::cout << __PRETTY_FUNCTION__ << "\n";
}
template <typename T, T v>
void pval()
{
    std::cout << __PRETTY_FUNCTION__ << "\n";
}

namespace bfc
{

template <std::size_t N, typename ReturnType, typename... Args>
class FixedFunctionObject
{
public:
FixedFunctionObject() = default;

FixedFunctionObject(const FixedFunctionObject& pOther)
{
    pOther.mCopier(mObject, pOther.mObject);
    set(pOther);
}

FixedFunctionObject(FixedFunctionObject&& pOther)
{
    std::memcpy(mObject, pOther.mObject, N);
    set(pOther);
    pOther.clear();
}

template <typename CallableObj, std::enable_if_t<!std::is_same_v<std::remove_reference_t<CallableObj>, FixedFunctionObject>>* p = nullptr>
FixedFunctionObject(CallableObj&& pObj)
{
    set(std::forward<CallableObj>(pObj));
}

FixedFunctionObject& operator=(const FixedFunctionObject& pOther)
{
    reset();
    pOther.mCopier(mObject, pOther.mObject);
    set(pOther);
}

FixedFunctionObject& operator=(FixedFunctionObject&& pOther)
{
    reset();
    std::memcpy(mObject, pOther.mObject, N);
    set(pOther);
    pOther.clear();
    return *this;
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

operator bool()
{
    return mFn;
}

void reset()
{
    if (mDestroyer)
    {
        mDestroyer(mObject);
    }
    clear();
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
    template <typename CallableObj, std::enable_if_t<!std::is_same_v<std::remove_reference_t<CallableObj>, FixedFunctionObject>>* p = nullptr>
    void set(CallableObj&& pObj)
    {
        using CallableObjType = std::decay_t<CallableObj>;
        static_assert(N>=sizeof(CallableObjType));
        new (mObject) CallableObjType(std::forward<CallableObjType>(pObj));
        mDestroyer = [](void* pObj)
        {
            ((CallableObjType*)pObj)->~CallableObjType();
        };

        mCopier = [](void* pObj, const void* pOther)
        {
            new (pObj) CallableObjType(*(const CallableObjType*)pOther);
        };

        mFn = [](void* pObj, Args&&... pArgs) -> ReturnType
        {
            return (*((CallableObjType*)pObj))(std::forward<Args>(pArgs)...);
        };
    }

    void set(const FixedFunctionObject& pOther)
    {
        mFn = pOther.mFn;
        mDestroyer = pOther.mDestroyer;
        mCopier = pOther.mCopier;
    }

    void set(const std::nullptr_t)
    {
        clear();
    }

    void clear()
    {
        mFn = nullptr;
        mDestroyer = nullptr;
        mCopier = nullptr;
    }

    uint8_t mObject[N];
    ReturnType (*mFn)(void*, Args&&...) = nullptr;
    void (*mDestroyer)(void*) = nullptr;
    void (*mCopier)(void*, const void*) = nullptr;
};

template <std::size_t N, typename T> struct FixedFunctionObjectTypeHelper;
template <std::size_t N, typename ReturnType, typename... ArgsType> struct FixedFunctionObjectTypeHelper<N, ReturnType(ArgsType...)>
{
    using type = FixedFunctionObject<N, ReturnType, ArgsType...>;
};

template <typename FnType>
using LightFunctionObject = typename FixedFunctionObjectTypeHelper<32, FnType>::type;

} // namespace bfc

#endif // __FIXEDFUNCTIONOBJECT_HPP__