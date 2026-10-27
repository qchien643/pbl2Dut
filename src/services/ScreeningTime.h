#pragma once

#include "entities/Screening.h"

#include <cstdint>
#include <string>

namespace screening_time {

std::int64_t minutes(const std::string& value);
std::int64_t endMinute(const Screening& screening, int buffer = 0);
std::string formatMinute(std::int64_t value);
void validateDate(const std::string& date);

}
