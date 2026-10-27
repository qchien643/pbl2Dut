#include "entities/Seat.h"
#include "entities/Auditorium.h"
#include "entities/EntityValidation.h"

Seat::Seat(EntityId id, const Auditorium& auditorium,
           std::int64_t rowNumber, std::int64_t seatNumber)
{
    this->id_ = entity_detail::positive(id, "Seat ID");
    this->auditorium_ = &auditorium;
    this->rowNumber_ = entity_detail::positive(rowNumber, "Row number");
    this->seatNumber_ = entity_detail::positive(seatNumber, "Seat number");
}

EntityId Seat::id() const
{
    return this->id_;
}

EntityId Seat::auditoriumId() const
{
    return this->auditorium_->id();
}

const Auditorium& Seat::auditorium() const
{
    return *this->auditorium_;
}

std::int64_t Seat::rowNumber() const
{
    return this->rowNumber_;
}

std::int64_t Seat::seatNumber() const
{
    return this->seatNumber_;
}
