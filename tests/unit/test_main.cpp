#include <gtest/gtest.h>

// Basic test to verify testing framework works
TEST(BasicTest, SanityCheck) {
    EXPECT_TRUE(true);
    EXPECT_EQ(2 + 2, 4);
}

// This main is provided by gtest_main, but we could customize it if needed
// int main(int argc, char** argv) {
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }