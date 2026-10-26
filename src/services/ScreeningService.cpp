#include "services/ScreeningService.h"

#include "services/ScreeningTime.h"
#include "services/ScreeningValidation.h"

#include <stdexcept>

ScreeningService::ScreeningService(const FilmRepository& films,
                                   const AuditoriumRepository& rooms,
                                   ScreeningRepository& screenings,
                                   const TicketRepository& tickets, ServiceClock clock)
    : films_(films), rooms_(rooms), screenings_(screenings), tickets_(tickets), clock_(clock)
{
    if (clock_ == nullptr)
    {
        throw std::invalid_argument("Clock is required");
    }
}

const Screening& ScreeningService::find(EntityId id) const
{
    if (id <= 0)
    {
        throw std::invalid_argument("ID must be positive");
    }
    const Screening* screening = screenings_.findById(id);
    if (screening == nullptr)
    {
        throw std::out_of_range("Record not found");
    }
    return *screening;
}

std::string ScreeningService::endTime(EntityId id) const
{
    const Screening& screening = find(id);
    const std::int64_t end = screening_time::endMinute(screening, films_);
    return screening_time::formatMinute(end);
}

void ScreeningService::validate(const Screening& candidate) const
{
    service_validation::screening(candidate, films_, rooms_, screenings_);
    const std::int64_t start = screening_time::minutes(candidate.startTime());
    const std::string currentTime = clock_();
    const std::int64_t now = screening_time::minutes(currentTime);
    if (start <= now)
    {
        throw std::invalid_argument("Screening must start in the future");
    }
}

EntityId ScreeningService::add(EntityId filmId, EntityId roomId,
                               std::string startTime, std::int64_t price)
{
    const EntityId id = screenings_.nextId();
    const Screening candidate(id, filmId, roomId, startTime, price);
    validate(candidate);
    screenings_.insert(candidate);
    return id;
}

void ScreeningService::requireNoTickets(EntityId id) const
{
    find(id);
    for (const Ticket& ticket : tickets_)
    {
        if (ticket.screeningId() == id)
        {
            throw std::invalid_argument("Screening already has tickets");
        }
    }
}

void ScreeningService::list(const std::string& date, EntityId filmId, EntityId roomId,
                           bool futureOnly, LinkedList<Screening>& result) const
{
    validateFilters(date, filmId, roomId);
    std::string now;
    if (futureOnly)
    {
        now = clock_();
        entity_detail::dateTime(now);
    }
    LinkedList<Screening> matches;
    for (const Screening& screening : screenings_)
    {
        if (!date.empty() && screening.startTime().compare(0, 10, date) != 0)
        {
            continue;
        }
        if (filmId != 0 && screening.filmId() != filmId)
        {
            continue;
        }
        if (roomId != 0 && screening.auditoriumId() != roomId)
        {
            continue;
        }
        if (futureOnly && screening.startTime() <= now)
        {
            continue;
        }
        matches.pushBack(screening);
    }
    result.swap(matches);
}

void ScreeningService::update(EntityId id, EntityId filmId, EntityId roomId,
                              std::string startTime, std::int64_t price)
{
    requireNoTickets(id);
    const Screening candidate(id, filmId, roomId, startTime, price);
    validate(candidate);
    screenings_.update(id, candidate);
}

void ScreeningService::erase(EntityId id)
{
    requireNoTickets(id);
    screenings_.erase(id);
}

void ScreeningService::validateFilters(const std::string& date,
                                       EntityId filmId, EntityId roomId)
{
    if (!date.empty())
    {
        screening_time::validateDate(date);
    }
    if (filmId < 0 || roomId < 0)
    {
        throw std::invalid_argument("Filter IDs must not be negative");
    }
}
