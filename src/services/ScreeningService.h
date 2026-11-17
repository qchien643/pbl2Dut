#pragma once

#include "entities/EntityValidation.h"
#include "entities/Auditorium.h"
#include "entities/Film.h"
#include "entities/Screening.h"
#include "repositories/IRepository.h"
#include "repositories/ITicketRepository.h"
#include "services/ServiceClock.h"

#include <cstdint>
#include <string>

class ScreeningService
{
public:
    ScreeningService(const IRepository<Film>& films, const IRepository<Auditorium>& rooms,
                     IRepository<Screening>& screenings, const ITicketRepository& tickets,
                     ServiceClock clock = localMinuteNow);

    EntityId add(EntityId filmId, EntityId roomId, std::string startTime, std::int64_t price);
    void update(EntityId id, EntityId filmId, EntityId roomId,
                std::string startTime, std::int64_t price);
    // Sold tickets preserve their screening; elapsed time does not lift this rule.
    void erase(EntityId id);
    const Screening& find(EntityId id) const;
    std::string endTime(EntityId id) const;

    // Empty date and zero IDs mean no filter. Replace result with matching records.
    void list(const std::string& date, EntityId filmId, EntityId roomId,
              bool futureOnly, LinkedList<Screening>& result) const;

private:
    void validate(const Screening& candidate) const;
    void requireNoTickets(EntityId id) const;
    static void validateFilters(const std::string& date, EntityId filmId, EntityId roomId);

    const IRepository<Film>& films_;
    const IRepository<Auditorium>& rooms_;
    IRepository<Screening>& screenings_;
    const ITicketRepository& tickets_;
    ServiceClock clock_;
};
