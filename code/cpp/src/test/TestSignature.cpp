#include<string>

#include<gtest/gtest.h>

#include "Cut.hpp"
#include "ValarrayUtils.hpp"

using namespace cut;

TEST(SignatureOperators, Equals) {
    valarrutils::ValarrayEqual<SizeType> eq;
    ASSERT_TRUE(eq(Signature({1, 2}), Signature({1, 2})));
    ASSERT_TRUE(eq(Signature({1, 2, 3}), Signature({1, 2, 3})));
    ASSERT_TRUE(eq(Signature({0, 0}), Signature({0, 0})));
    ASSERT_FALSE(eq(Signature({2, 1}), Signature({1, 2})));
}

TEST(SignatureOperators, Plus) {
    valarrutils::ValarrayEqual<SizeType> eq;
    Signature sig({1, 2, 3});
    Signature old_sig({1, 2 , 3});
    sig += Signature({1, 2, 3});
    ASSERT_TRUE(eq(sig, Signature({2, 4, 6})));
    ASSERT_TRUE(eq(sig, old_sig + old_sig));
}
