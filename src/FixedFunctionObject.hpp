#ifndef __FIXEDFUNCTIONOBJECT_HPP__
#define __FIXEDFUNCTIONOBJECT_HPP__

#include <cstddef>
#include <cstring>
#include <functional>

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
    pOther.mMover(mObject, pOther.mObject);
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
    pOther.mMover(mObject, pOther.mObject);
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

operator bool() const
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

ReturnType operator()(Args&&... pArgs) const
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

        mMover = [](void* pObj, void* pOther)
        {
            new (pObj) CallableObjType(std::move(*(CallableObjType*)pOther));
        };

        mFn = [](const void* pObj, Args&&... pArgs) -> ReturnType
        {
            return (*((CallableObjType*)pObj))(std::forward<Args>(pArgs)...);
        };
    }

    void set(const FixedFunctionObject& pOther)
    {
        mFn = pOther.mFn;
        mDestroyer = pOther.mDestroyer;
        mCopier = pOther.mCopier;
        mMover = pOther.mMover;
    }

    void set(const std::nullptr_t)
    {
        clear();
    }

    void clear()
    {
        mFn = nullptr;
    }

    uint8_t mObject[N];
    ReturnType (*mFn)(const void*, Args&&...) = nullptr;
    void (*mDestroyer)(void*) = nullptr;
    void (*mCopier)(void*, const void*) = nullptr;
    void (*mMover)(void*, void*) = nullptr;
};

template <std::size_t N, typename T> struct FixedFunctionObjectTypeHelper;
template <std::size_t N, typename ReturnType, typename... ArgsType> struct FixedFunctionObjectTypeHelper<N, ReturnType(ArgsType...)>
{
    using type = FixedFunctionObject<N, ReturnType, ArgsType...>;
};

template <typename FnType> using UltraLightFunctionObject = typename FixedFunctionObjectTypeHelper<8, FnType>::type;
template <typename FnType> using LightFunctionObject = typename FixedFunctionObjectTypeHelper<24, FnType>::type;
template <typename FnType> using HeavyFunctionObject = typename FixedFunctionObjectTypeHelper<32, FnType>::type;

} // namespace bfc

#endif // __FIXEDFUNCTIONOBJECT_HPP__