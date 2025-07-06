#include <gtest/gtest.h>

#include <bfc/buffer.hpp>

using namespace bfc;

TEST(buffer_view, ShouldConstructFromPointer_NonConstToConst)
{
    std::byte* v{};
    size_t s{};
    const_buffer_view(v,s);
}
TEST(buffer_view, ShouldConstructFromPointer_NonConstToNonConst)
{
    std::byte* v{};
    size_t s{};
    buffer_view(v,s);
}
TEST(buffer_view, ShouldConstructFromPointer_ConstToConst)
{
    const std::byte* v{};
    size_t s{};
    const_buffer_view(v,s);
}
TEST(buffer_view, ShouldConstructFrom_buffer_view_NonConstToConst)
{
    std::byte* v{};
    size_t s{};
    const_buffer_view(v,s);
}
TEST(buffer_view, ShouldConstructFrom_buffer_view_NonConstToNonConst)
{
    buffer_view(buffer_view{});
}
TEST(buffer_view, ShouldConstructFrom_buffer_view_ConstToConst)
{
    const_buffer_view(const_buffer_view{});
}
TEST(buffer_view, ShouldConstructFrom_buffer_NonConstToConst)
{
    const_buffer_view(buffer{});
}
TEST(buffer_view, ShouldConstructFrom_buffer_NonConstToNonConst)
{
    buffer_view(buffer{});
}
TEST(buffer_view, ShouldConstructFrom_buffer_ConstToConst)
{
    const_buffer_view(const_buffer{});
}
TEST(buffer_view, ShouldAssignFrom_buffer_view_NonConstToConst)
{
    const_buffer_view bv;
    bv = buffer{};
}
TEST(buffer_view, ShouldAssignFrom_buffer_view_NonConstToNonConst)
{
    buffer_view bv;
    bv = buffer_view{};
}
TEST(buffer_view, ShouldAssignFrom_buffer_view_ConstToConst)
{
    const_buffer_view bv;
    bv = buffer_view{};
}
TEST(buffer_view, ShouldAssignFrom_buffer_NonConstToConst)
{
    const_buffer_view bv;
    bv = buffer{};
}
TEST(buffer_view, ShouldAssignFrom_buffer_NonConstToNonConst)
{
    buffer_view bv;
    bv = buffer{};
}
TEST(buffer_view, ShouldAssignFrom_buffer_ConstToConst)
{
    const_buffer_view bv;
    bv = const_buffer{};
}
TEST(buffer_view, ShouldAssignConstruct_vector_NonConstToNonConst)
{
    buffer_view(std::vector<std::byte>{});
}
TEST(buffer_view, ShouldNotConstructFrom_vector_NonConstToConst)
{
    const_buffer_view(std::vector<std::byte>{});
}
TEST(buffer_view, ShouldAssignFrom_vector_NonConstToNonConst)
{
    buffer_view bv;
    bv = std::vector<std::byte>{};
}
TEST(buffer_view, ShouldNotAssignFrom_vector_NonConstToConst)
{
    const_buffer_view bv;
    bv = std::vector<std::byte>{};
}
TEST(buffer_view, ShouldNotConstructFromPointer_ConstToNonConst)
{
    // @note: Should Not Compile
    // const std::byte* v{};
    // size_t   s{};
    // buffer_view(v,s);
}
TEST(buffer_view, ShouldNotConstructFrom_buffer_view_ConstToNonConst)
{
    // @note: Should Not Compile
    // buffer_view(const_buffer_view{});
}
TEST(buffer_view, ShouldNotConstructFrom_buffer_ConstToNonConst)
{
    // @note: Should Not Compile
    // buffer_view(const_buffer{});
}
TEST(buffer_view, ShouldNotAssignFrom_buffer_view_ConstToNonConst)
{
    // @note: Should Not Compile
    // buffer_view bv;
    // bv = const_buffer_view{};
}
TEST(buffer_view, ShouldNotAssignFrom_buffer_ConstToNonConst)
{
    // @note: Should Not Compile
    // buffer_view bv;
    // bv = const_buffer{};
}
