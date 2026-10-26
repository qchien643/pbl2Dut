#include "services/ScreeningValidation.h"

#include "services/ScreeningTime.h"

#include <stdexcept>

void service_validation::screening(const Screening& candidate, const FilmRepository& films,
                                   const AuditoriumRepository& rooms,
                                   const ScreeningRepository& screenings)
{
    const Auditorium* room = rooms.findById(candidate.auditoriumId());
    if (room == nullptr)
    {
        throw std::out_of_range("Record not found");
    }
    const std::int64_t start = screening_time::minutes(candidate.startTime());
    const std::int64_t end = screening_time::endMinute(candidate, films, 15);
    for (const Screening& item : screenings)
    {
        if (item.id() == candidate.id())
        {
            continue;
        }
        if (item.auditoriumId() != candidate.auditoriumId())
        {
            continue;
        }
        const std::int64_t otherEnd = screening_time::endMinute(item, films, 15);
        if (start >= otherEnd)
        {
            continue;
        }
        const std::int64_t otherStart = screening_time::minutes(item.startTime());
        if (otherStart < end)
        {
            throw std::invalid_argument("Screening overlaps another room booking");
        }
    }
}
