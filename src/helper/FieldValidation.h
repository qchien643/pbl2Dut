#pragma once

#include <cstdint>
#include <string>

namespace field_validation
{
    std::int64_t requirePositiveNumber(std::int64_t value, const char* field);
    std::string validateAndTrimName(const std::string& value);
}
