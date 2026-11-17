#include "services/ScreeningValidation.h"

#include "services/ScreeningTime.h"

#include <stdexcept>

void service_validation::screening(const Screening& candidate,
                                   const IRepository<Screening>& screenings)
{
    const std::int64_t start = screening_time::minutes(candidate.startTime());
    const std::int64_t end = screening_time::endMinute(candidate, 15);
    LinkedList<const Screening*> records;
    screenings.list(records);
    for (const Screening* record : records)
    {
        const Screening& item = *record;
        if (item.id() == candidate.id())
        {
            continue;
        }
        if (&item.auditorium() != &candidate.auditorium())
        {
            continue;
        }
        const std::int64_t otherEnd = screening_time::endMinute(item, 15);
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
