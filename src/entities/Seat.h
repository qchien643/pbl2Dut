#pragma once

#include "entities/EntityId.h"

#include <cstdint>

class Auditorium;

class Seat
{
public:
    Seat(EntityId id, const Auditorium& auditorium,
         std::int64_t rowNumber, std::int64_t seatNumber);

    EntityId id() const;
    EntityId auditoriumId() const;
    const Auditorium& auditorium() const;
    std::int64_t rowNumber() const;
    std::int64_t seatNumber() const;

private:
    EntityId id_;
    const Auditorium* auditorium_;
    std::int64_t rowNumber_;
    std::int64_t seatNumber_;
};
