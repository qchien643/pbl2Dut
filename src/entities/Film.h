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
    const std::u32string& normalizedTitle() const;
    std::int64_t durationMinutes() const;

private:
    EntityId id_;
    std::string title_;
    std::u32string normalizedTitle_;
    std::int64_t durationMinutes_;
};
