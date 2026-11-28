#include "entities/Film.h"
#include "helper/FieldValidation.h"
#include "helper/TextNormalization.h"

Film::Film(EntityId id, std::string title, std::int64_t durationMinutes)
{
    this->id_ = field_validation::requirePositiveNumber(id, "Film ID");
    this->title_ = field_validation::validateAndTrimName(title);
    this->normalizedTitle_ = text_normalization::normalize(this->title_);
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

const std::u32string& Film::normalizedTitle() const
{
    return this->normalizedTitle_;
}

std::int64_t Film::durationMinutes() const
{
    return this->durationMinutes_;
}
