#include<iostream>
#include<vector>
#include<string>

#include<gtest/gtest.h>

int main(int argc, char** argv) {
    size_t argc_s = static_cast<size_t>(argc);
    char* argv_unquoted_c[argc_s];
    std::vector<std::string> argv_unquoted(argc_s);
    for (size_t i = 0; i < argc_s; ++i) {
        argv_unquoted[i] = argv[i]; 
        if (argv_unquoted[i].front() == '\'' && argv_unquoted[i].back() == '\'') {
            argv_unquoted[i].erase(0);
            argv_unquoted[i].pop_back();
        }
        argv_unquoted_c[i] = const_cast<char*>(argv_unquoted[i].c_str());
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
