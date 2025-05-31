#include <gtest/gtest.h>

#include <bfc/command_manager.hpp>

using namespace bfc;

TEST(configuration_parser, shoulParseConfig)
{
    configuration_parser parser;
    parser.load_line("key1 = 1234");
    parser.load_line("key2 = 12.34");
    parser.load_line("key3 = qwerty");

    EXPECT_EQ(1234,  parser.as<int>("key1"));
    EXPECT_EQ(4321,  parser.as<int>("key12").value_or(4321));
    EXPECT_EQ(12.34, parser.as<double>("key2"));
    EXPECT_EQ("qwerty", parser.arg("key3"));
    EXPECT_FALSE(parser.arg("key31"));
    EXPECT_FALSE(parser.as<int>("key3"));
}
