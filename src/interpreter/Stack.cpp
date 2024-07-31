#include "Stack.h"
#include "ast/FunctionDefinitionNode.h"

Stack::Stack() : data(), variableData(), stackPointer(0) {}

Stack::~Stack() {}


uint8_t *Stack::pop_front(const size_t elementSize)
{

    auto result = &data[stackPointer];
    stackPointer += elementSize;
    return result;
}

void Stack::set_var(std::string &varName, const int64_t &value)
{
    constexpr size_t size = sizeof(int64_t);

    if (has_var(varName))
    {
        auto pointer = &variableData[m_variables[varName]];
        std::memcpy(pointer, &value, size);
    }
    else
    {
        m_variables[varName] = variableData.size();
        size_t oldStart = variableData.size();
        variableData.resize(variableData.size() + size);
        auto pointer = &variableData[oldStart];

        std::memcpy(pointer, &value, size);
    }
}

uint8_t *Stack::get_var_raw(const std::string &varName) { return &variableData[m_variables[varName]]; }


void Stack::set_var(std::string &varName, const std::string_view value)
{
    if (has_var(varName))
    {
        auto pointer = &variableData[m_variables[varName]];
        *pointer = value.size();
        pointer += sizeof(int64_t);
        for (auto c: value)
        {
            *pointer = c;
            pointer += sizeof(char);
        }
    }
    else
    {
        m_variables[varName] = variableData.size();

        {
            size_t oldStart = variableData.size();
            auto size = sizeof(int64_t);
            variableData.resize(variableData.size() + size);
            auto pointer = &variableData[oldStart];
            size_t v = value.size();
            std::memcpy(pointer, &v, size);
        }

        for (auto c: value)
            variableData.push_back(c);
    }
}


bool Stack::has_var(const std::string &varName) { return m_variables.count(varName) != 0; }

void Stack::addFunction(std::shared_ptr<FunctionDefinitionNode> &function) { m_functions[function->name()] = function; }
void Stack::addFunction(FunctionDefinitionNode *function)
{
    m_functions[function->name()] = std::shared_ptr<FunctionDefinitionNode>(function);
}

std::shared_ptr<FunctionDefinitionNode> &Stack::getFunction(const std::string &name) { return m_functions.at(name); }
