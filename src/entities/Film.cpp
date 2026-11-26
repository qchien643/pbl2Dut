#include "entities/Film.h"
#include "helper/FieldValidation.h"

Film::Film(EntityId id, std::string title, std::int64_t durationMinutes)
{
    this->id_ = field_validation::requirePositiveNumber(id, "Film ID");
    this->title_ = field_validation::validateAndTrimName(title);
    this->durationMinutes_ = field_validation::requirePositiveNumber(durationMinutes, "Duration");
}

EntityId Film::id() const
{
    return this->id_;
}

const std::string& Film::title() const
{
    return this->title_;
}

std::int64_t Film::durationMinutes() const
{
    return this->durationMinutes_;
}
