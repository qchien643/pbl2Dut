#include "entities/Auditorium.h"
#include "helper/FieldValidation.h"

#include <limits>
#include <stdexcept>

Auditorium::Auditorium(EntityId id, std::string name,
                       std::int64_t rowCount, std::int64_t seatsPerRow)
{
    this->id_ = field_validation::requirePositiveNumber(id, "Auditorium ID");
    this->name_ = field_validation::validateAndTrimName(name);
    this->rowCount_ = field_validation::requirePositiveNumber(rowCount, "Row count");
    this->seatsPerRow_ = field_validation::requirePositiveNumber(seatsPerRow, "Seats per row");

    const std::int64_t maximum = std::numeric_limits<std::int64_t>::max();
    if (this->rowCount_ > maximum / this->seatsPerRow_)
    {
        throw std::overflow_error("Auditorium capacity overflow");
    }
}

EntityId Auditorium::id() const
{
    return this->id_;
}

const std::string& Auditorium::name() const
{
    return this->name_;
}

std::int64_t Auditorium::rowCount() const
{
    return this->rowCount_;
}

std::int64_t Auditorium::seatsPerRow() const
{
    return this->seatsPerRow_;
}

std::int64_t Auditorium::capacity() const
{
    return this->rowCount_ * this->seatsPerRow_;
}
