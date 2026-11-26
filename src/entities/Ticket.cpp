#include "entities/Ticket.h"
#include "helper/FieldValidation.h"
#include "entities/Screening.h"
#include "entities/Seat.h"
#include "helper/DateTimeHelper.h"

#include <stdexcept>

Ticket::Ticket(EntityId id, const Screening& screening, const Seat& seat,
               std::string soldAt, std::int64_t unitPrice)
{
    this->id_ = field_validation::requirePositiveNumber(id, "Ticket ID");
    if (&seat.auditorium() != &screening.auditorium())
    {
        throw std::invalid_argument("Ticket seat belongs to wrong room");
    }
    this->screening_ = &screening;
    this->seat_ = &seat;
    this->soldAt_ = date_time::validateDateTime(soldAt);
    this->unitPrice_ = field_validation::requirePositiveNumber(unitPrice, "Unit price");
}

EntityId Ticket::id() const
{
    return this->id_;
}

EntityId Ticket::screeningId() const
{
    return this->screening_->id();
}

EntityId Ticket::seatId() const
{
    return this->seat_->id();
}

const Screening& Ticket::screening() const
{
    return *this->screening_;
}

const Seat& Ticket::seat() const
{
    return *this->seat_;
}

void Ticket::setSeat(const Seat& seat) noexcept
{
    this->seat_ = &seat;
}

const std::string& Ticket::soldAt() const
{
    return this->soldAt_;
}

std::int64_t Ticket::unitPrice() const
{
    return this->unitPrice_;
}
