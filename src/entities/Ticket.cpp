#include "entities/Ticket.h"
#include "entities/EntityValidation.h"

Ticket::Ticket(EntityId id, EntityId screeningId, EntityId seatId,
               std::string soldAt, std::int64_t unitPrice)
{
    this->id_ = entity_detail::positive(id, "Ticket ID");
    this->screeningId_ = entity_detail::positive(screeningId, "Screening ID");
    this->seatId_ = entity_detail::positive(seatId, "Seat ID");
    this->soldAt_ = entity_detail::dateTime(soldAt);
    this->unitPrice_ = entity_detail::positive(unitPrice, "Unit price");
}

EntityId Ticket::id() const
{
    return this->id_;
}

EntityId Ticket::screeningId() const
{
    return this->screeningId_;
}

EntityId Ticket::seatId() const
{
    return this->seatId_;
}

const std::string& Ticket::soldAt() const
{
    return this->soldAt_;
}

std::int64_t Ticket::unitPrice() const
{
    return this->unitPrice_;
}
