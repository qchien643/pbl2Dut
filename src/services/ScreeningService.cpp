#include "services/ScreeningService.h"

#include "helper/DateTimeHelper.h"

#include <stdexcept>

ScreeningService::ScreeningService(const IFilmRepository& films,
                                   const IRepository<Auditorium>& rooms,
                                   IRepository<Screening>& screenings,
                                   const IRepository<Ticket>& tickets, date_time::Clock clock)
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
    const std::int64_t start = date_time::toMinutes(screening.startTime());
    const std::int64_t end = date_time::addMinutes(start, screening.film().durationMinutes());
    return date_time::fromMinutes(end);
}

void ScreeningService::validate(const Screening& candidate) const
{
    const int cleanupMinutes = 15;
    const std::int64_t start = date_time::toMinutes(candidate.startTime());
    const std::int64_t end = date_time::addMinutes(
        date_time::addMinutes(start, candidate.film().durationMinutes()), cleanupMinutes);
    LinkedList<const Screening*> records;
    screenings_.list(records);
    for (const Screening* record : records)
    {
        const Screening& item = *record;
        if (item.id() == candidate.id() || &item.auditorium() != &candidate.auditorium())
        {
            continue;
        }
        const std::int64_t otherStart = date_time::toMinutes(item.startTime());
        const std::int64_t otherEnd = date_time::addMinutes(
            date_time::addMinutes(otherStart, item.film().durationMinutes()), cleanupMinutes);
        if (start < otherEnd && otherStart < end)
        {
            throw std::invalid_argument("Screening overlaps another room booking");
        }
    }
    const std::string currentTime = clock_();
    const std::int64_t now = date_time::toMinutes(currentTime);
    if (start <= now)
    {
        throw std::invalid_argument("Screening must start in the future");
    }
}

EntityId ScreeningService::add(EntityId filmId, EntityId roomId,
                               std::string startTime, std::int64_t price)
{
    if (filmId <= 0 || roomId <= 0)
    {
        throw std::invalid_argument("ID must be positive");
    }
    const Film* film = films_.findById(filmId);
    const Auditorium* room = rooms_.findById(roomId);
    if (film == nullptr || room == nullptr)
    {
        throw std::out_of_range("Record not found");
    }
    const EntityId id = screenings_.nextId();
    const Screening candidate(id, *film, *room, startTime, price);
    validate(candidate);
    screenings_.insert(candidate);
    return id;
}

void ScreeningService::requireNoTickets(EntityId id) const
{
    const Screening& screening = find(id);
    LinkedList<const Ticket*> records;
    tickets_.list(records);
    for (const Ticket* record : records)
    {
        const Ticket& ticket = *record;
        if (&ticket.screening() == &screening)
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
        date_time::validateDateTime(now);
    }
    LinkedList<Screening> matches;
    LinkedList<const Screening*> records;
    screenings_.list(records);
    for (const Screening* record : records)
    {
        const Screening& screening = *record;
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
    if (filmId <= 0 || roomId <= 0)
    {
        throw std::invalid_argument("ID must be positive");
    }
    const Film* film = films_.findById(filmId);
    const Auditorium* room = rooms_.findById(roomId);
    if (film == nullptr || room == nullptr)
    {
        throw std::out_of_range("Record not found");
    }
    const Screening candidate(id, *film, *room, startTime, price);
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
        date_time::validateDate(date);
    }
    if (filmId < 0 || roomId < 0)
    {
        throw std::invalid_argument("Filter IDs must not be negative");
    }
}
