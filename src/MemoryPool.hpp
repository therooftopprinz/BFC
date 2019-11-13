#ifndef __MEMORYPOOL_HPP__
#define __MEMORYPOOL_HPP__

#include <mutex>
#include <array>
#include <cmath>
#include <vector>
#include <cstddef>

#include <bfc/FixedFunctionObject.hpp>
#include <bfc/Buffer.hpp>

namespace bfc
{

template <std::size_t ALIGNMENT=alignof(std::max_align_t)>
class SizedMemoryPool
{
public:
    SizedMemoryPool(const std::size_t pSize)
        : mSize(pSize)
    {}

    ~SizedMemoryPool()
    {
        for (auto i : mAllocations)
        {
            operator delete[](i, std::align_val_t {ALIGNMENT});
        }
    }

    Buffer allocate()
    {
        std::byte* rv;
        std::unique_lock<std::mutex> lg(mAllocationMutex);
        if (mAllocations.size())
        {
            rv = mAllocations.back();
            mAllocations.pop_back();
            lg.unlock();
        }
        else
        {
            rv = (std::byte*) operator new[](mSize, std::align_val_t {ALIGNMENT});
        }
        return Buffer(rv, mSize, [this](std::byte* pPtr){free(pPtr);});
    }

    std::size_t allocationSize() const
    {
        return mSize;
    }

private:
    void free(std::byte* pPtr)
    {
        std::unique_lock<std::mutex> lg(mAllocationMutex);
        mAllocations.emplace_back(pPtr);
    }

    const std::size_t mSize;
    std::vector<std::byte*> mAllocations;
    std::mutex mAllocationMutex;
};

template <std::size_t ALIGNMENT=alignof(std::max_align_t)>
class Log2MemoryPool
{
public:
    Buffer allocate(std::size_t pSize)
    {
        if (pSize<=8)
        {
            throw std::bad_alloc();
        }
        int index = std::ceil(std::log2(pSize))-4;
        return mPools.at(index).allocate();
    }

private:
    using Pool = SizedMemoryPool<ALIGNMENT>;
    std::array<Pool,11> mPools = {
        Pool(16),
        Pool(32),
        Pool(64),
        Pool(128),
        Pool(256),
        Pool(512),
        Pool(1024),
        Pool(2048),
        Pool(4096),
        Pool(8192),
        Pool(16384)
    };
};

} // namespace bfc

#endif // __MEMORYPOOL_HPP__
