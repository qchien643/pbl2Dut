#include "services/ServiceClock.h"

#include "entities/EntityValidation.h"

#include <ctime>
#include <stdexcept>

std::string localMinuteNow() {
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
    return entity_detail::dateTime(text);
}
