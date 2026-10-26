#include "entities/Screening.h"
#include "entities/EntityValidation.h"

Screening::Screening(EntityId id, EntityId filmId, EntityId auditoriumId,
                     std::string startTime, std::int64_t ticketPrice)
{
    this->id_ = entity_detail::positive(id, "Screening ID");
    this->filmId_ = entity_detail::positive(filmId, "Film ID");
    this->auditoriumId_ = entity_detail::positive(auditoriumId, "Auditorium ID");
    this->startTime_ = entity_detail::dateTime(startTime);
    this->ticketPrice_ = entity_detail::positive(ticketPrice, "Ticket price");
}

EntityId Screening::id() const
{
    return this->id_;
}

EntityId Screening::filmId() const
{
    return this->filmId_;
}

EntityId Screening::auditoriumId() const
{
    return this->auditoriumId_;
}

const std::string& Screening::startTime() const
{
    return this->startTime_;
}

std::int64_t Screening::ticketPrice() const
{
    return this->ticketPrice_;
}
