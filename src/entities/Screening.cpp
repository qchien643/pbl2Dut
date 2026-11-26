#include "entities/Screening.h"
#include "entities/Auditorium.h"
#include "helper/FieldValidation.h"
#include "entities/Film.h"
#include "helper/DateTimeHelper.h"

Screening::Screening(EntityId id, const Film& film, const Auditorium& auditorium,
                     std::string startTime, std::int64_t ticketPrice)
{
    this->id_ = field_validation::requirePositiveNumber(id, "Screening ID");
    this->film_ = &film;
    this->auditorium_ = &auditorium;
    this->startTime_ = date_time::validateDateTime(startTime);
    this->ticketPrice_ = field_validation::requirePositiveNumber(ticketPrice, "Ticket price");
}

EntityId Screening::id() const
{
    return this->id_;
}

EntityId Screening::filmId() const
{
    return this->film_->id();
}

EntityId Screening::auditoriumId() const
{
    return this->auditorium_->id();
}

const Film& Screening::film() const
{
    return *this->film_;
}

const Auditorium& Screening::auditorium() const
{
    return *this->auditorium_;
}

const std::string& Screening::startTime() const
{
    return this->startTime_;
}

std::int64_t Screening::ticketPrice() const
{
    return this->ticketPrice_;
}
