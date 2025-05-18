#include <gtest/gtest.h>

#include <bfc/buffer.hpp>

using namespace bfc;

TEST(buffer_view, ShouldConstructFromPointer_NonConstToConst)
{
    EXPECT_TRUE ((std::is_constructible<const_buffer_view,uint8_t*, size_t>::value));
}
TEST(buffer_view, ShouldConstructFromPointer_NonConstToNonConst)
{
    EXPECT_TRUE ((std::is_constructible<buffer_view,uint8_t*, size_t>::value));
}
TEST(buffer_view, ShouldConstructFromPointer_ConstToConst)
{
    EXPECT_TRUE ((std::is_constructible<const_buffer_view,const uint8_t*, size_t>::value));
}
TEST(buffer_view, ShouldNotConstructFromPointer_ConstToNonConst)
{
    EXPECT_FALSE((std::is_constructible<buffer_view,const uint8_t*, size_t>::value));
}


TEST(buffer_view, ShouldConstructFrombuffer_view_NonConstToConst)
{
    EXPECT_TRUE ((std::is_constructible<const_buffer_view,buffer_view>::value));
}
TEST(buffer_view, ShouldConstructFrombuffer_view_NonConstToNonConst)
{
    EXPECT_TRUE ((std::is_constructible<buffer_view,buffer_view>::value));
}
TEST(buffer_view, ShouldConstructFrombuffer_view_ConstToConst)
{
    EXPECT_TRUE ((std::is_constructible<const_buffer_view,const_buffer_view>::value));
}
TEST(buffer_view, ShouldNotConstructFrombuffer_view_ConstToNonConst)
{
    EXPECT_FALSE((std::is_constructible<buffer_view,const_buffer_view>::value));
}


TEST(buffer_view, ShouldConstructFromBuffer_NonConstToConst)
{
    EXPECT_TRUE ((std::is_constructible<const_buffer_view,buffer>::value));
}
TEST(buffer_view, ShouldConstructFromBuffer_NonConstToNonConst)
{
    EXPECT_TRUE ((std::is_constructible<buffer_view,buffer>::value));
}
TEST(buffer_view, ShouldConstructFromBuffer_ConstToConst)
{
    EXPECT_TRUE ((std::is_constructible<const_buffer_view,const_buffer>::value));
}
TEST(buffer_view, ShouldNotConstructFromBuffer_ConstToNonConst)
{
    EXPECT_FALSE((std::is_constructible<buffer_view,const_buffer>::value));
}


TEST(buffer_view, ShouldAssignFrombuffer_view_NonConstToConst)
{
    EXPECT_TRUE ((std::is_assignable<const_buffer_view,buffer_view>::value));
}
TEST(buffer_view, ShouldAssignFrombuffer_view_NonConstToNonConst)
{
    EXPECT_TRUE ((std::is_assignable<buffer_view,buffer_view>::value));
}
TEST(buffer_view, ShouldAssignFrombuffer_view_ConstToConst)
{
    EXPECT_TRUE ((std::is_assignable<const_buffer_view,const_buffer_view>::value));
}
TEST(buffer_view, ShouldNotAssignFrombuffer_view_ConstToNonConst)
{
    EXPECT_FALSE((std::is_assignable<buffer_view,const_buffer_view>::value));
}


// TEST(buffer_view, ShouldAssignFromBuffer_NonConstToConst)
// {
//     EXPECT_TRUE ((std::is_assignable<const_buffer_view,Buffer>::value));
// }
// TEST(buffer_view, ShouldAssignFromBuffer_NonConstToNonConst)
// {
//     EXPECT_TRUE ((std::is_assignable<buffer_view,Buffer>::value));
// }
// TEST(buffer_view, ShouldAssignFromBuffer_ConstToConst)
// {
//     EXPECT_TRUE ((std::is_assignable<const_buffer_view,ConstBuffer>::value));
// }
TEST(buffer_view, ShouldNotAssignFromBuffer_ConstToNonConst)
{
    EXPECT_FALSE((std::is_assignable<buffer_view,const_buffer>::value));
}