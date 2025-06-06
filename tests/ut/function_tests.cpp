#include <gtest/gtest.h>

#include <bfc/function.hpp>

using namespace bfc;

struct TestClass
{
    static int count;
    static int copy;
    static int move;
    static int called;
    TestClass()
    {
        // std::cout << __PRETTY_FUNCTION__ << "\n";
        count++;
    }
    TestClass(const TestClass&)
    {
        // std::cout << __PRETTY_FUNCTION__ << "\n";
        copy++;
        count++;
    }
    TestClass(TestClass&&)
    {
        // std::cout << __PRETTY_FUNCTION__ << "\n";
        move++;
        count++;
    }
    ~TestClass()
    {
        // std::cout << __PRETTY_FUNCTION__ << "\n";
        count--;
    }
    static void reset()
    {
        count = 0;
        copy = 0;
        move = 0;
        called = 0;
    }

    void operator()()
    {
        called++;
    }

    int increment(int pV)
    {
        return pV+1;
    }
    void add (int& a, int b)
    {
        a += b;
    }
};

int TestClass::count = 0;
int TestClass::copy = 0;
int TestClass::move = 0;
int TestClass::called = 0;

int increment(int pV)
{
    return pV+1;
}

TEST(FixedFunctionObject, ShouldCallMemberFunction)
{
    TestClass tc;
    light_function<int(int)> fn([&tc](int i)->int {return tc.increment(i);});
    EXPECT_EQ(42, fn(41));
}

TEST(FixedFunctionObject, ShouldCallNonMemberFunction)
{
    light_function<int(int)> fn(increment);
    EXPECT_EQ(42, fn(41));
}

TEST(FixedFunctionObject, ShouldCallMemberFunctionMixedArgs)
{
    TestClass tc;
    light_function<void(int&,int)> fn([&tc](int& a, int b)->void {return tc.add(a,b);});
    int a = 41;
    fn(a,1);
    EXPECT_EQ(42, a);
}

TEST(FixedFunctionObject, ShouldCallCallable)
{
    TestClass::reset();
    light_function<void()> fn{TestClass()};
    fn();
    EXPECT_EQ(1, TestClass::called);
}

TEST(FixedFunctionObject, ShouldCallDestruct)
{
    TestClass::reset();
    {
        light_function<void()> fn{TestClass()};
        EXPECT_EQ(1, TestClass::count);
    }
    EXPECT_EQ(0, TestClass::count);
}

TEST(FixedFunctionObject, ShouldCopyConstruct)
{
    TestClass::reset();
    light_function<void()> fn{TestClass()};
    light_function<void()> fn2(fn);
    EXPECT_EQ(1, TestClass::copy);
}

TEST(FixedFunctionObject, ShouldMoveConstruct)
{
    TestClass::reset();
    light_function<void()> fn{TestClass()};
    light_function<void()> fn2(std::move(fn));
    EXPECT_EQ(2, TestClass::move);
}


TEST(FixedFunctionObject, ShouldThrowUnset)
{
    light_function<void()> fn;
    EXPECT_THROW(fn(), std::bad_function_call);
}

TEST(FixedFunctionObject, ShouldSetEqNullptr)
{
    light_function<void()> fn{TestClass()};
    fn = nullptr;
    EXPECT_THROW(fn(), std::bad_function_call);
}

TEST(FixedFunctionObject, ShouldConstructNullptr)
{
    light_function<void()> fn{nullptr};
    EXPECT_THROW(fn(), std::bad_function_call);
}

TEST(FixedFunctionObject, ShouldCopyConstructAndAssignFromEmpty)
{
    light_function<void()> fn;
    light_function<void()> fn2(fn);
    light_function<void()> fn3;
    fn3 = fn;
    EXPECT_THROW(fn2(), std::bad_function_call);
    EXPECT_THROW(fn3(), std::bad_function_call);
}

TEST(FixedFunctionObject, ShouldMoveConstructAndAssignFromEmpty)
{
    light_function<void()> fn;
    light_function<void()> fn2(std::move(fn));
    light_function<void()> fn3;
    fn = std::move(fn2);
    EXPECT_THROW(fn2(), std::bad_function_call);
    EXPECT_THROW(fn3(), std::bad_function_call);
}

TEST(FixedFunctionObject, ShouldAssignFromStaticMember)
{
    light_function<void()> fn = &TestClass::reset;

    TestClass::count = 1;

    fn();

    EXPECT_EQ(0, TestClass::count);
}
