#include <gtest/gtest.h>
#include <BFC/FixedFunctionObject.hpp>

using namespace bfc;

struct TestClass
{
    int increment(int pV)
    {
        return pV+1;
    }
    void add (int& a, int b)
    {
        a += b;
    }
};

int increment(int pV)
{
    return pV+1;
}

TEST(FixedFunctionObject, ShoulCallMemberFunction)
{
    TestClass tc;
    LightFunctionObject<int(int)> fn([&tc](int i)->int {return tc.increment(i);});
    EXPECT_EQ(42, fn(41));
}

TEST(FixedFunctionObject, ShoulCallNonMemberFunction)
{
    LightFunctionObject<int(int)> fn(increment);
    EXPECT_EQ(42, fn(41));
}

TEST(FixedFunctionObject, ShoulCallMemberFunctionMixedArgs)
{
    TestClass tc;
    LightFunctionObject<void(int&,int)> fn([&tc](int& a, int b)->void {return tc.add(a,b);});
    int a = 41;
    fn(a,1);
    EXPECT_EQ(42, a);
}

TEST(FixedFunctionObject, ShoulThrowUnset)
{
    LightFunctionObject<void()> fn;
    EXPECT_THROW(fn(), std::bad_function_call);
}
