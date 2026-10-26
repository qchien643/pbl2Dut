#pragma once

#include "entities/EntityId.h"

#include <cstdint>

class Seat
{
public:
    Seat(EntityId id, EntityId auditoriumId, std::int64_t rowNumber, std::int64_t seatNumber);

    EntityId id() const;
    EntityId auditoriumId() const;
    std::int64_t rowNumber() const;
    std::int64_t seatNumber() const;

private:
    EntityId id_;
    EntityId auditoriumId_;
    std::int64_t rowNumber_;
    std::int64_t seatNumber_;
};
