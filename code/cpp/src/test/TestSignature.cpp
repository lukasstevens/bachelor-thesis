#include<string>

#include<gtest/gtest.h>

#include "Partition.hpp"

using namespace part;

TEST(SignatureHash, LengthOne) {
    std::hash<Signature> sig_hash_fn;
    std::hash<std::string> string_hash_fn;

    std::string s("aaaa");
    unsigned long s_as_int = 0;
    for (auto c : s) {
        s_as_int += static_cast<unsigned int>(c);
        s_as_int <<= 8;
    }
    s_as_int >>= 8;

    Signature sig(std::vector<Signature::CountType>({static_cast<unsigned int>(s_as_int)}));
    ASSERT_EQ(string_hash_fn(s), sig_hash_fn(sig));
}

TEST(SignatureHash, LengthTwo) {
    std::hash<Signature> sig_hash_fn;
    std::hash<std::string> string_hash_fn;

    std::string s;
    for (size_t i = 0; i < 2; ++i) {
        s.push_back('a');
        for (size_t k = 0; k < 3; ++k) {
            s.push_back('\0');
        }
    }

    Signature sig(std::vector<Signature::CountType>({97, 97}));
    ASSERT_EQ(string_hash_fn(s), sig_hash_fn(sig));
}

TEST(SignatureOperators, Equals) {
    ASSERT_EQ(Signature({1, 2}), Signature({1, 2}));
    ASSERT_EQ(Signature({1, 2, 3}), Signature({1, 2, 3}));
    ASSERT_EQ(Signature({0, 0}), Signature({0, 0}));
    ASSERT_NE(Signature({2, 1}), Signature({1, 2}));
}

TEST(SignatureOperators, Plus) {
    Signature sig({1, 2, 3});
    Signature old_sig({1, 2 , 3});
    sig += Signature({1, 2, 3});
    ASSERT_EQ(sig, Signature({2, 4, 6}));
    ASSERT_EQ(sig, old_sig + old_sig);
}
