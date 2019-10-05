#include <gtest/gtest.h>
#include <BFC/CommandManager.hpp>

using namespace bfc;

TEST(CommandManager, ShouldExecute)
{
    CommandManager cmdMan;
    cmdMan.executeCommand("window fd =1 trigger = manual size= 40");
}

TEST(CommandManager, ShouldFillArgs)
{
    CommandManager cmdMan;
    int fd;
    std::string trigger;
    int size;
    CommandManager::CmdFnCb fn = [&](ArgsMap&& pArgs) -> std::string
    {
        fd = *pArgs.argAs<int>("fd");
        trigger = *pArgs.argAs<std::string>("trigger");
        size = *pArgs.argAs<int>("size");
        return "OK";
    };
    cmdMan.addCommand("window", fn);
    ASSERT_EQ("OK", cmdMan.executeCommand("window fd =1 trigger = manual size= 40"));
    EXPECT_EQ(1, fd);
    EXPECT_EQ("manual", trigger);
    EXPECT_EQ(40, size);
}