#pragma once

#include "entities/EntityId.h"

#include <cstdint>
#include <string>

class Film
{
public:
    Film(EntityId id, std::string title, std::int64_t durationMinutes);

    EntityId id() const;
    const std::string& title() const;
    std::int64_t durationMinutes() const;

private:
    EntityId id_;
    std::string title_;
    std::int64_t durationMinutes_;
};
