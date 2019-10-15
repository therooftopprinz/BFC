#ifndef __MEMORYPOOL_HPP__
#define __MEMORYPOOL_HPP__

#include <mutex>
#include <array>
#include <cmath>
#include <vector>

#include <BFC/FixedFunctionObject.hpp>
#include <BFC/Buffer.hpp>

namespace bfc
{

// TODO: Allocation alignment on pool
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
            delete[] (uint8_t*)i;
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
        }
        else
        {
            rv = new std::byte[mSize];
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
    std::array<SizedMemoryPool,11> mPools = {
        SizedMemoryPool(16),
        SizedMemoryPool(32),
        SizedMemoryPool(64),
        SizedMemoryPool(128),
        SizedMemoryPool(256),
        SizedMemoryPool(512),
        SizedMemoryPool(1024),
        SizedMemoryPool(2048),
        SizedMemoryPool(4096),
        SizedMemoryPool(8192),
        SizedMemoryPool(16384)
    };
};

} // namespace bfc

#endif // __MEMORYPOOL_HPP__
