#include<string>

#include<gtest/gtest.h>

#include "Partition.hpp"
#include "ValarrayUtils.hpp"

using namespace part;

TEST(SignatureHash, LengthTwo) {
    valarrutils::ValarrayHasher<SizeType> sig_hash_fn;
    std::hash<std::string> string_hash_fn;

    std::string s;
    for (size_t i = 0; i < 2; ++i) {
        s.push_back('a');
        for (size_t k = 0; k < 3; ++k) {
            s.push_back('\0');
        }
    }

    Signature sig({97, 97});
    ASSERT_EQ(string_hash_fn(s), sig_hash_fn(sig));
}

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
