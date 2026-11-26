#include "helper/DateTimeHelper.h"

#include <ctime>
#include <iomanip>
#include <locale>
#include <sstream>
#include <stdexcept>

namespace {

int monthDays(int year, int month) {
    const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return days[month - 1]
        + (month == 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

}

std::string date_time::validateDateTime(const std::string& value)
{
    if (value.size() != 16)
    {
        throw std::invalid_argument("Date must use YYYY-MM-DD HH:mm");
    }
    if (value[4] != '-' || value[7] != '-' || value[10] != ' ' || value[13] != ':')
    {
        throw std::invalid_argument("Date must use YYYY-MM-DD HH:mm");
    }
    for (std::size_t index = 0; index < value.size(); index++)
    {
        if (index == 4 || index == 7 || index == 10 || index == 13)
        {
            continue;
        }
        if (value[index] < '0' || value[index] > '9')
        {
            throw std::invalid_argument("Date contains a non-digit");
        }
    }

    const int year = std::stoi(value.substr(0, 4));
    const int month = std::stoi(value.substr(5, 2));
    const int day = std::stoi(value.substr(8, 2));
    const int hour = std::stoi(value.substr(11, 2));
    const int minute = std::stoi(value.substr(14, 2));
    if (year == 0 || month < 1 || month > 12 || hour > 23 || minute > 59)
    {
        throw std::invalid_argument("Date or time is out of range");
    }

    if (day < 1 || day > monthDays(year, month))
    {
        throw std::invalid_argument("Day is out of range");
    }
    return value;
}

// ponytail: local calendar minutes only; add timezone/DST support if scope expands.
std::int64_t date_time::toMinutes(const std::string& value) {
    validateDateTime(value);
    const int year = std::stoi(value.substr(0, 4));
    const int month = std::stoi(value.substr(5, 2));
    const std::int64_t previousYear = year - 1;
    std::int64_t days = previousYear * 365 + previousYear / 4
        - previousYear / 100 + previousYear / 400;
    for (int current = 1; current < month; ++current) {
        days += monthDays(year, current);
    }
    days += std::stoi(value.substr(8, 2)) - 1;
    return days * 1440 + std::stoi(value.substr(11, 2)) * 60 + std::stoi(value.substr(14, 2));
}

std::int64_t date_time::addMinutes(std::int64_t value, std::int64_t amount) {
    static const std::int64_t maximum = toMinutes("9999-12-31 23:59");
    if (value < 0 || value > maximum) {
        throw std::out_of_range("Time is outside supported date range");
    }
    if (amount < -value || amount > maximum - value) {
        throw std::overflow_error("Time addition exceeds supported date range");
    }
    return value + amount;
}

std::string date_time::fromMinutes(std::int64_t value) {
    addMinutes(value, 0); // Check the supported range before converting.
    std::int64_t days = value / 1440;
    int year = 1;
    while (days >= 365 + (monthDays(year, 2) == 29)) {
        days -= 365 + (monthDays(year, 2) == 29);
        ++year;
    }
    int month = 1;
    while (days >= monthDays(year, month)) {
        days -= monthDays(year, month++);
    }
    std::ostringstream result;
    result.imbue(std::locale::classic());
    result << std::setfill('0') << std::setw(4) << year << '-'
           << std::setw(2) << month << '-' << std::setw(2) << days + 1 << ' '
           << std::setw(2) << value % 1440 / 60 << ':' << std::setw(2) << value % 60;
    return result.str();
}

void date_time::validateDate(const std::string& date) {
    validateDateTime(date + " 00:00");
}

std::string date_time::localMinuteNow() {
    const std::time_t now = std::time(nullptr);
    if (now == std::time_t(-1)) {
        throw std::runtime_error("Cannot read local clock");
    }
    const std::tm* local = std::localtime(&now);
    if (local == nullptr) {
        throw std::runtime_error("Cannot read local clock");
    }
    char text[32];
    if (std::strftime(text, sizeof(text), "%Y-%m-%d %H:%M", local) == 0) {
        throw std::runtime_error("Cannot format local clock");
    }
    return validateDateTime(text);
}
