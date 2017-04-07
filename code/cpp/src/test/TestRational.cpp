#include<gtest/gtest.h>

#include "Rational.hpp"

using namespace rat;

TEST(Operators, Relational) {
    ASSERT_LT(Rational<int>(2, 3), Rational<int>(4, 5));
    ASSERT_LT(Rational<unsigned int>(2, 3), Rational<unsigned int>(4, 5));

    ASSERT_GT(Rational<int>(4, 5), Rational<int>(2, 3));
    ASSERT_GT(Rational<unsigned int>(4, 5), Rational<unsigned int>(2, 3));

    ASSERT_EQ(Rational<int>(2, 2), Rational<int>(-1, -1));
    ASSERT_EQ(Rational<int>(2, 4), Rational<int>(1, 2));

    ASSERT_EQ(Rational<unsigned int>(2, 4), Rational<unsigned int>(1, 2));
}


