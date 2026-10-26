#include "services/ScreeningTime.h"

#include "entities/EntityValidation.h"

#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace {

int monthDays(int year, int month) {
    const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return days[month - 1]
        + (month == 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

}

// ponytail: local calendar minutes only; add timezone/DST support if scope expands.
std::int64_t screening_time::minutes(const std::string& value) {
    entity_detail::dateTime(value);
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

std::int64_t screening_time::endMinute(const Screening& screening,
                                       const FilmRepository& films, int buffer) {
    const std::int64_t start = minutes(screening.startTime());
    const Film* film = films.findById(screening.filmId());
    if (film == nullptr)
    {
        throw std::out_of_range("Record not found");
    }
    const std::int64_t duration = film->durationMinutes();
    static const std::int64_t maximum = minutes("9999-12-31 23:59");
    if (duration > maximum - start - buffer) {
        throw std::overflow_error("Screening end exceeds supported date range");
    }
    return start + duration + buffer;
}

std::string screening_time::formatMinute(std::int64_t value) {
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
    result << std::setfill('0') << std::setw(4) << year << '-'
           << std::setw(2) << month << '-' << std::setw(2) << days + 1 << ' '
           << std::setw(2) << value % 1440 / 60 << ':' << std::setw(2) << value % 60;
    return result.str();
}

void screening_time::validateDate(const std::string& date) {
    entity_detail::dateTime(date + " 00:00");
}
