#include <gtest/gtest.h>
#include <Buffer.hpp>

using namespace bfc;

TEST(BufferView, ShouldConstructFromPointer_NonConstToConst)
{
    EXPECT_TRUE ((std::is_constructible<ConstBufferView,uint8_t*, size_t>::value));
}
TEST(BufferView, ShouldConstructFromPointer_NonConstToNonConst)
{
    EXPECT_TRUE ((std::is_constructible<BufferView,uint8_t*, size_t>::value));
}
TEST(BufferView, ShouldConstructFromPointer_ConstToConst)
{
    EXPECT_TRUE ((std::is_constructible<ConstBufferView,const uint8_t*, size_t>::value));
}
TEST(BufferView, ShouldNotConstructFromPointer_ConstToNonConst)
{
    EXPECT_FALSE((std::is_constructible<BufferView,const uint8_t*, size_t>::value));
}


TEST(BufferView, ShouldConstructFromBufferView_NonConstToConst)
{
    EXPECT_TRUE ((std::is_constructible<ConstBufferView,BufferView>::value));
}
TEST(BufferView, ShouldConstructFromBufferView_NonConstToNonConst)
{
    EXPECT_TRUE ((std::is_constructible<BufferView,BufferView>::value));
}
TEST(BufferView, ShouldConstructFromBufferView_ConstToConst)
{
    EXPECT_TRUE ((std::is_constructible<ConstBufferView,ConstBufferView>::value));
}
TEST(BufferView, ShouldNotConstructFromBufferView_ConstToNonConst)
{
    EXPECT_FALSE((std::is_constructible<BufferView,ConstBufferView>::value));
}


TEST(BufferView, ShouldConstructFromBuffer_NonConstToConst)
{
    EXPECT_TRUE ((std::is_constructible<ConstBufferView,Buffer>::value));
}
TEST(BufferView, ShouldConstructFromBuffer_NonConstToNonConst)
{
    EXPECT_TRUE ((std::is_constructible<BufferView,Buffer>::value));
}
TEST(BufferView, ShouldConstructFromBuffer_ConstToConst)
{
    EXPECT_TRUE ((std::is_constructible<ConstBufferView,ConstBuffer>::value));
}
TEST(BufferView, ShouldNotConstructFromBuffer_ConstToNonConst)
{
    EXPECT_FALSE((std::is_constructible<BufferView,ConstBuffer>::value));
}


TEST(BufferView, ShouldAssignFromBufferView_NonConstToConst)
{
    EXPECT_TRUE ((std::is_assignable<ConstBufferView,BufferView>::value));
}
TEST(BufferView, ShouldAssignFromBufferView_NonConstToNonConst)
{
    EXPECT_TRUE ((std::is_assignable<BufferView,BufferView>::value));
}
TEST(BufferView, ShouldAssignFromBufferView_ConstToConst)
{
    EXPECT_TRUE ((std::is_assignable<ConstBufferView,ConstBufferView>::value));
}
TEST(BufferView, ShouldNotAssignFromBufferView_ConstToNonConst)
{
    EXPECT_FALSE((std::is_assignable<BufferView,ConstBufferView>::value));
}


// TEST(BufferView, ShouldAssignFromBuffer_NonConstToConst)
// {
//     EXPECT_TRUE ((std::is_assignable<ConstBufferView,Buffer>::value));
// }
// TEST(BufferView, ShouldAssignFromBuffer_NonConstToNonConst)
// {
//     EXPECT_TRUE ((std::is_assignable<BufferView,Buffer>::value));
// }
// TEST(BufferView, ShouldAssignFromBuffer_ConstToConst)
// {
//     EXPECT_TRUE ((std::is_assignable<ConstBufferView,ConstBuffer>::value));
// }
TEST(BufferView, ShouldNotAssignFromBuffer_ConstToNonConst)
{
    EXPECT_FALSE((std::is_assignable<BufferView,ConstBuffer>::value));
}