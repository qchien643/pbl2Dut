#pragma once

#include <cstdint>
#include <string>

namespace date_time {

using Clock = std::string (*)();
std::string localMinuteNow();

// Supported years: 0001..9999. Minute zero is 0001-01-01 00:00.
std::string validateDateTime(const std::string& value);
void validateDate(const std::string& date);
std::int64_t toMinutes(const std::string& value);
std::string fromMinutes(std::int64_t value);
std::int64_t addMinutes(std::int64_t value, std::int64_t amount);

}
