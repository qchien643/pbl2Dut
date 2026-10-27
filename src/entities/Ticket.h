#pragma once

#include "entities/EntityId.h"

#include <cstdint>
#include <string>

class Screening;
class Seat;

class Ticket
{
public:
    Ticket(EntityId id, const Screening& screening, const Seat& seat,
           std::string soldAt, std::int64_t unitPrice);

    EntityId id() const;
    EntityId screeningId() const;
    EntityId seatId() const;
    const Screening& screening() const;
    const Seat& seat() const;
    const std::string& soldAt() const;
    std::int64_t unitPrice() const;

private:
    friend class TicketRepository;
    void setSeat(const Seat& seat) noexcept;

    EntityId id_;
    const Screening* screening_;
    const Seat* seat_;
    std::string soldAt_;
    std::int64_t unitPrice_;
};
