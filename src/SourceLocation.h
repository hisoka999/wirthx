#pragma once

#include <memory>
#include <string>
struct SourceLocation
{
    std::string filename;
    std::shared_ptr<std::string> source;
    std::size_t byte_offset;
    std::size_t num_bytes;

    [[nodiscard]] std::string text() const { return source->substr(byte_offset, num_bytes); }
};
