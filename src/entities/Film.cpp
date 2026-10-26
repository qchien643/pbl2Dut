#include "entities/Film.h"
#include "entities/EntityValidation.h"

Film::Film(EntityId id, std::string title, std::int64_t durationMinutes)
{
    this->id_ = entity_detail::positive(id, "Film ID");
    this->title_ = entity_detail::name(title);
    this->durationMinutes_ = entity_detail::positive(durationMinutes, "Duration");
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
