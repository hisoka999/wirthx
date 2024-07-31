#pragma once
#include <cstring>
#include <vector>


struct PascalIntArray
{
public:
    PascalIntArray() : m_low(0), m_height(0) {}

    PascalIntArray(size_t low, size_t height) : m_low(low), m_height(height)
    {
        m_data.resize(m_height - m_low + 1);

        for (size_t i = 0; i < size(); i++)
        {
            m_data[i] = 0;
        }
    }

    PascalIntArray(PascalIntArray &other) : m_low(other.m_low), m_height(other.m_height)
    {
        m_data.resize(m_height - m_low + 1);
        std::memcpy(m_data.data(), other.m_data.data(), sizeof(int64_t) * size());
    }

    PascalIntArray(size_t low, size_t height, uint8_t *data) : m_low(low), m_height(height)
    {
        m_data.resize(m_height - m_low + 1);
        for (size_t i = 0; i < size(); i++)
        {
            int64_t value;
            std::memcpy(&value, data, sizeof(int64_t));
            m_data[i] = value;
            data += sizeof(int64_t);
        }
    }

    int64_t &operator[](std::size_t idx) { return m_data[idx - m_low]; }


    size_t low() { return m_low; }
    size_t height() { return m_height; }
    size_t size() { return m_height - m_low + 1; }


    int64_t *data() { return m_data.data(); }

private:
    size_t m_low;
    size_t m_height;
    std::vector<int64_t> m_data;
};
