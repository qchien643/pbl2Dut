#include "entities/Seat.h"
#include "entities/EntityValidation.h"

Seat::Seat(EntityId id, EntityId auditoriumId,
           std::int64_t rowNumber, std::int64_t seatNumber)
{
    this->id_ = entity_detail::positive(id, "Seat ID");
    this->auditoriumId_ = entity_detail::positive(auditoriumId, "Auditorium ID");
    this->rowNumber_ = entity_detail::positive(rowNumber, "Row number");
    this->seatNumber_ = entity_detail::positive(seatNumber, "Seat number");
}

EntityId Seat::id() const
{
    return this->id_;
}

EntityId Seat::auditoriumId() const
{
    return this->auditoriumId_;
}

std::int64_t Seat::rowNumber() const
{
    return this->rowNumber_;
}

std::int64_t Seat::seatNumber() const
{
    return this->seatNumber_;
}
