#pragma once

#include "entities/EntityId.h"

#include <cstdint>
#include <string>

class Screening
{
public:
    Screening(EntityId id, EntityId filmId, EntityId auditoriumId,
              std::string startTime, std::int64_t ticketPrice);

    EntityId id() const;
    EntityId filmId() const;
    EntityId auditoriumId() const;
    const std::string& startTime() const;
    std::int64_t ticketPrice() const;

private:
    EntityId id_;
    EntityId filmId_;
    EntityId auditoriumId_;
    std::string startTime_;
    std::int64_t ticketPrice_;
};

