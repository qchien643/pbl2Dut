#pragma once

#include <cstdint>
#include <string>

namespace entity_detail
{
    std::int64_t positive(std::int64_t value, const char* field);
    std::string name(const std::string& value);
    std::string dateTime(const std::string& value);
}
