#include<iostream>

#include<gtest/gtest.h>

int main(int argc, char** argv) {
    std::cerr << "gtest cli args:";
    for (int arg_idx = 0; arg_idx < argc; ++arg_idx) {
        std::cerr << " " << argv[arg_idx];
    }
    std::cerr << std::endl;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
