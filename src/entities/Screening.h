#pragma once

#include "entities/EntityId.h"

#include <cstdint>
#include <string>

class Auditorium;
class Film;

class Screening
{
public:
    Screening(EntityId id, const Film& film, const Auditorium& auditorium,
              std::string startTime, std::int64_t ticketPrice);

    EntityId id() const;
    EntityId filmId() const;
    EntityId auditoriumId() const;
    const Film& film() const;
    const Auditorium& auditorium() const;
    const std::string& startTime() const;
    std::int64_t ticketPrice() const;

private:
    EntityId id_;
    const Film* film_;
    const Auditorium* auditorium_;
    std::string startTime_;
    std::int64_t ticketPrice_;
};

