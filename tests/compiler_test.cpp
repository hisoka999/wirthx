#include "compiler/Compiler.h"
#include "os/command.h"
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <utility>

using namespace std::literals;

class CompilerTest : public testing::TestWithParam<std::string>
{
};

TEST_P(CompilerTest, TestNoError)
{
    // Inside a test, access the test parameter with the GetParam() method
    // of the TestWithParam<T> class:
    auto name = GetParam();
    std::filesystem::path input_path = "testfiles/" + name + ".pas";
    std::filesystem::path output_path = "testfiles/" + name + ".txt";
    ASSERT_TRUE(std::filesystem::exists(input_path));
    ASSERT_TRUE(std::filesystem::exists(output_path));
    std::stringstream ostream;
    std::stringstream erstream;
    compile_file(input_path, erstream, std::cout);

    std::ifstream file;
    std::istringstream is;
    std::string s;
    std::string group;

    execute_command(ostream, (std::filesystem::current_path() / name).string());

    file.open(output_path, std::ios::in);

    if (!file.is_open())
    {
        std::cerr << input_path.c_str() << "\n";
        std::cerr << std::filesystem::absolute(input_path);
        FAIL();
    }
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string expected(size, ' ');
    file.seekg(0);
    file.read(&expected[0], size);
    std::cout << "expected: " << expected;
    std::cout << ostream.str() << "\n";
    ASSERT_EQ(erstream.str(), "");
    ASSERT_EQ(ostream.str(), expected);
    ASSERT_GT(ostream.str().size(), 0);
}

INSTANTIATE_TEST_SUITE_P(CompilerTestNoError,
                         CompilerTest,
                         testing::Values("helloworld", "functions", "math", "includetest", "whileloop", "conditions", "forloop"));