#include <gtest/gtest.h>
#include "ecs/ECSCore.hpp"

TEST(ECSCoreTest, RegistryCreated) {
    ASSERT_NO_THROW({
        transity::ecs::ECSCore ecsCore;
        ecsCore.initialize();
    });
}