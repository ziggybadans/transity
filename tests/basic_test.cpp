#include <gtest/gtest.h>

TEST(BasicTest, BasicAssertions) {
    EXPECT_STRNE("hello", "world");
    EXPECT_EQ(1, 1);
}

TEST(BasicTest, AlwaysPasses) {
    EXPECT_TRUE(true);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}