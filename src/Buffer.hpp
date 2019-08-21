#pragma once

#include <type_traits>

namespace bfc
{

template<typename T>
class BufferImpl
{
public:
    BufferImpl(T* pData,size_t pSize)
        : mSize(pSize)
        , mData(pData)
    {
    }
    BufferImpl(const BufferImpl&) = delete;
    void operator=(const BufferImpl&) = delete;
    ~BufferImpl()
    {
        if (mData)
        {
            delete[] mData;
        }
    }
    T* data() const
    {
        return mData;
    }
    size_t size() const
    {
        return mSize;
    }
private:
    size_t mSize = 0;
    T* mData = nullptr;
};

using Buffer = BufferImpl<uint8_t>;
using ConstBuffer = BufferImpl<const uint8_t>;

template<typename T>
class BufferViewImpl
{
    using NCT = typename std::remove_const<T>::type;
    using CT = typename std::add_const<NCT>::type;
public:
    /* Constructors
    ** NCT -> CT  OK
    ** NCT -> NCT OK
    ** CT  -> CT  OK
    ** CT  -> NCT NOK
    */ 
    template<template<class> class U, class V>
    BufferViewImpl(const U<V>& pBuffer,
        typename std::enable_if<std::is_same<V,NCT>::value||std::is_same<V,T>::value>::type* = 0)
        : mSize(pBuffer.size())
        , mData(pBuffer.data())
    {}

    template <typename U>
    BufferViewImpl(U data, size_t size,
        typename std::enable_if<std::is_same<U,NCT*>::value||std::is_same<U,T*>::value>::type* = 0)
        : mSize(size)
        , mData(data)
    {}

    template<template<class> class U, class V>
    typename std::enable_if<std::is_same<V,NCT>::value||std::is_same<V,T>::value,BufferViewImpl>::type&
        operator=(U<V> pBuffer)
    {
        mSize = pBuffer.size();
        mData = pBuffer.data();
        return *this;
    }

    T* data() const
    {
        return mData;
    }

    size_t size() const
    {
        return mSize;
    }
private:
    size_t mSize = 0;
    T* mData = nullptr;
};

using BufferView = BufferViewImpl<uint8_t>;
using ConstBufferView = BufferViewImpl<const uint8_t>;

}