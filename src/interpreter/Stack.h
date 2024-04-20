#pragma once

#include <vector>
#include <cstdint>
#include <variant>
#include <string>
#include <map>
#include <memory>

typedef std::variant<std::string_view, int64_t> StackObject;

class FunctionDefinitionNode;

class Stack
{
private:
    std::vector<StackObject> data;
    size_t stackPointer;
    std::map<std::string, StackObject> m_variables;
    std::map<std::string, std::shared_ptr<FunctionDefinitionNode>> m_functions;

public:
    Stack(/* args */);
    ~Stack();
    void push_back(const std::string_view &value);
    void push_back(const int64_t value);
    void push_back(const StackObject &value);
    void set_var(std::string &varName, const std::string_view &value);
    void set_var(std::string &varName, const int64_t &value);
    void set_var(std::string &varName, const StackObject &value);
    void addFunction(std::shared_ptr<FunctionDefinitionNode> &function);
    void addFunction(FunctionDefinitionNode *function);
    std::shared_ptr<FunctionDefinitionNode> &getFunction(const std::string &name);
    StackObject get_var(const std::string &varName);
    template <typename T>

    T get_var(std::string &varName)
    {
        auto value = get_var(varName);
        return std::get<T>(value);
    }
    StackObject pop_front();
    template <typename T>
    T pop_front()
    {
        auto value = pop_front();
        return std::get<T>(value);
    }
};