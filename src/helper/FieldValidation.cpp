#include "helper/FieldValidation.h"

#include <stdexcept>

std::int64_t field_validation::requirePositiveNumber(std::int64_t value, const char* field)
{
    if (value <= 0)
    {
        throw std::invalid_argument(std::string(field) + " must be positive");
    }
    return value;
}

std::string field_validation::validateAndTrimName(const std::string& value)
{
    for (unsigned char character : value)
    {
        if (character < 32 || character == 127 || character == '|')
        {
            throw std::invalid_argument("Name contains a control character or delimiter");
        }
    }

    const std::size_t first = value.find_first_not_of(' ');
    if (first == std::string::npos)
    {
        throw std::invalid_argument("Name must not be empty");
    }
    const std::size_t last = value.find_last_not_of(' ');
    const std::size_t length = last - first + 1;
    return value.substr(first, length);
}
