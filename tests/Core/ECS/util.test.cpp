#include <Core/ECS/util.h>
#include <gtest/gtest.h>

using namespace Core;

TEST(ECS_UTIL_TEST, creates_indices_for_Components)
{
    ASSERT_EQ(type_index<double>::value(), 0);
    ASSERT_EQ(type_index<int>::value(), 1);
    ASSERT_EQ(type_index<char>::value(), 2);
    ASSERT_EQ(type_index<int>::value(), 1);
}