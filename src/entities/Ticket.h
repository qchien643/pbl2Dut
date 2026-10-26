#pragma once

#include "entities/EntityId.h"

#include <cstdint>
#include <string>

class Ticket
{
public:
    Ticket(EntityId id, EntityId screeningId, EntityId seatId,
           std::string soldAt, std::int64_t unitPrice);

    EntityId id() const;
    EntityId screeningId() const;
    EntityId seatId() const;
    const std::string& soldAt() const;
    std::int64_t unitPrice() const;

private:
    EntityId id_;
    EntityId screeningId_;
    EntityId seatId_;
    std::string soldAt_;
    std::int64_t unitPrice_;
};
