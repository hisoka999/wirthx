#pragma once

#include <bit>
#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include "PascalArray.h"

class FunctionDefinitionNode;

class Stack
{
private:
    std::vector<uint8_t> data;
    std::vector<uint8_t> variableData;
    size_t stackPointer;
    std::map<std::string, size_t> m_variables;
    std::map<std::string, std::shared_ptr<FunctionDefinitionNode>> m_functions;
    bool m_break = false;

private:
    uint8_t *get_var_raw(const std::string &varName);

public:
    Stack(/* args */);
    ~Stack();
    // void push_back(const std::string_view &value);
    // void push_back(const int64_t value);
    // void push_back(const uint8_t *value);
    template<typename T>
    void push_back(const T value)
    {
        if constexpr (std::is_same_v<T, std::string_view>)
        {
            push_back(value.size());

            for (auto c: value)
                data.push_back(c);
        }
        else
        {
            size_t oldStart = data.size();
            auto size = sizeof(T);
            data.resize(data.size() + size);
            auto pointer = &data[oldStart];

            std::memcpy(pointer, &value, size);
        }
    }
    void set_var(const std::string &varName, const std::string_view value);
    void set_var(const std::string &varName, const int64_t &value);
    void addFunction(std::shared_ptr<FunctionDefinitionNode> &function);
    void addFunction(FunctionDefinitionNode *function);
    std::shared_ptr<FunctionDefinitionNode> &getFunction(const std::string &name);


    void set_var(const std::string &varName, PascalIntArray &array)
    {
        uint8_t *pointer = nullptr;
        if (has_var(varName))
        {
            pointer = &variableData[m_variables[varName]];
        }
        else
        {
            m_variables[varName] = variableData.size();
            size_t oldStart = variableData.size();
            size_t size = sizeof(int64_t) * (array.size() + 2);
            variableData.resize(variableData.size() + size);

            pointer = &variableData[oldStart];
        }


        size_t low = array.low();
        std::memcpy(pointer, &low, sizeof(size_t));
        pointer += sizeof(size_t);
        size_t heigh = array.height();
        std::memcpy(pointer, &heigh, sizeof(size_t));
        pointer += sizeof(size_t);

        auto data = array.data();
        std::memcpy(pointer, data, sizeof(size_t) * array.size());
    }

    template<typename T>
    T get_var(std::string &varName)
    {
        if constexpr (std::is_same_v<T, std::string_view>)
        {
            auto value = get_var_raw(varName);
            size_t stringSize = (size_t)*value;
            value += sizeof(size_t);
            return std::string_view((char *)value, stringSize);
        }
        else if constexpr (std::is_same_v<T, PascalIntArray>)
        {
            auto value = get_var_raw(varName);
            size_t low;
            std::memcpy(&low, value, sizeof(size_t));
            value += sizeof(size_t);
            size_t heigh;
            std::memcpy(&heigh, value, sizeof(size_t));
            value += sizeof(size_t);

            return PascalIntArray(low, heigh, value);
        }
        else
        {
            static_assert(std::is_trivially_copyable<T>::value);

            auto value = get_var_raw(varName);

            T result;
            std::memcpy(&result, value, sizeof(T));
            return result;
        }
    }

    bool has_var(const std::string &varName);
    uint8_t *pop_front(const size_t elementSize = 1);
    template<typename T>
    T pop_front()
    {
        if constexpr (std::is_same_v<T, std::string_view>)
        {
            auto value = pop_front(0);
            size_t stringSize = (size_t)*value;
            value += sizeof(size_t);
            stackPointer += sizeof(size_t) + stringSize;
            return std::string_view((char *)value, stringSize);
        }
        else
        {
            auto value = pop_front(sizeof(T));

            T result;
            std::memcpy(&result, value, sizeof(T));
            return result;
        }
    }


    void startBreak() { m_break = true; }
    bool stopBreakIfActive()
    {
        bool old = m_break;
        if (m_break)
            m_break = false;
        return old;
    }
};
