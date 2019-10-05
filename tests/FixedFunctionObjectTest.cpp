#include <gtest/gtest.h>
#include <BFC/FixedFunctionObject.hpp>

using namespace bfc;

struct TestClass
{
    static int count;
    static int copy;
    static int called;
    TestClass()
    {
        count++;
    }
    TestClass(const TestClass&)
    {
        copy++;
    }
    TestClass(TestClass&&)
    {
        count++;
    }
    ~TestClass()
    {
        count--;
    }
    static void reset()
    {
        count = 0;
        copy = 0;
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
int TestClass::called = 0;

int increment(int pV)
{
    return pV+1;
}

TEST(FixedFunctionObject, ShouldCallMemberFunction)
{
    TestClass tc;
    LightFunctionObject<int(int)> fn([&tc](int i)->int {return tc.increment(i);});
    EXPECT_EQ(42, fn(41));
}

TEST(FixedFunctionObject, ShouldCallNonMemberFunction)
{
    LightFunctionObject<int(int)> fn(increment);
    EXPECT_EQ(42, fn(41));
}

TEST(FixedFunctionObject, ShouldCallMemberFunctionMixedArgs)
{
    TestClass tc;
    LightFunctionObject<void(int&,int)> fn([&tc](int& a, int b)->void {return tc.add(a,b);});
    int a = 41;
    fn(a,1);
    EXPECT_EQ(42, a);
}

TEST(FixedFunctionObject, ShouldCallCallable)
{
    TestClass::reset();
    LightFunctionObject<void()> fn{TestClass()};
    fn();
    EXPECT_EQ(1, TestClass::called);
}

TEST(FixedFunctionObject, ShouldCallDestruct)
{
    TestClass::reset();
    {
        LightFunctionObject<void()> fn{TestClass()};
        EXPECT_EQ(1, TestClass::count);
    }
    EXPECT_EQ(0, TestClass::count);
}

TEST(FixedFunctionObject, ShouldCopyConstruct)
{
    TestClass::reset();
    LightFunctionObject<void()> fn{TestClass()};
    LightFunctionObject<void()> fn2(fn);
    EXPECT_EQ(1, TestClass::copy);
}

TEST(FixedFunctionObject, ShouldThrowUnset)
{
    LightFunctionObject<void()> fn;
    EXPECT_THROW(fn(), std::bad_function_call);
}
