#include <gtest/gtest.h>

#include <BFC/MemoryPool.hpp>

using namespace bfc;

TEST(SizeMemoryPool, ShouldAllocate)
{
    SizedMemoryPool sizedPool(64);
    Buffer buffer = sizedPool.allocate();
    ASSERT_NE(nullptr, buffer.data());
    ASSERT_EQ(64u, buffer.size());
}

TEST(Log2MemoryPool, ShouldAllocate)
{
    Log2MemoryPool pool;
    ASSERT_EQ(16u,     pool.allocate(9).size());
    ASSERT_EQ(32u,     pool.allocate(17).size());
    ASSERT_EQ(64u,     pool.allocate(33).size());
    ASSERT_EQ(128u,    pool.allocate(65).size());
    ASSERT_EQ(256u,    pool.allocate(129).size());
    ASSERT_EQ(512u,    pool.allocate(257).size());
    ASSERT_EQ(1024u,   pool.allocate(513).size());
    ASSERT_EQ(2048u,   pool.allocate(1025).size());
    ASSERT_EQ(4096u,   pool.allocate(2049).size());
    ASSERT_EQ(8192u,   pool.allocate(4097).size());
    ASSERT_EQ(16384u,  pool.allocate(8193).size());
    ASSERT_NE(nullptr, pool.allocate(9).data());
    ASSERT_NE(nullptr, pool.allocate(17).data());
    ASSERT_NE(nullptr, pool.allocate(33).data());
    ASSERT_NE(nullptr, pool.allocate(65).data());
    ASSERT_NE(nullptr, pool.allocate(129).data());
    ASSERT_NE(nullptr, pool.allocate(257).data());
    ASSERT_NE(nullptr, pool.allocate(513).data());
    ASSERT_NE(nullptr, pool.allocate(1025).data());
    ASSERT_NE(nullptr, pool.allocate(2049).data());
    ASSERT_NE(nullptr, pool.allocate(4097).data());
    ASSERT_NE(nullptr, pool.allocate(8193).data());
}
