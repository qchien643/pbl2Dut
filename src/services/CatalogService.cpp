#include "services/CatalogService.h"

#include <limits>
#include <stdexcept>

CatalogService::CatalogService(FilmRepository& films, AuditoriumRepository& rooms,
                               SeatRepository& seats, const ScreeningRepository& screenings,
                               TicketRepository& tickets)
    : films_(films), rooms_(rooms), seats_(seats), screenings_(screenings), tickets_(tickets)
{
}

EntityId CatalogService::addFilm(std::string title, std::int64_t duration)
{
    const EntityId id = films_.nextId();
    const Film candidate(id, title, duration);
    films_.insert(candidate);
    return id;
}

bool CatalogService::filmReferenced(EntityId id) const
{
    for (const Screening& screening : screenings_)
    {
        if (screening.filmId() == id)
        {
            return true;
        }
    }
    return false;
}

void CatalogService::films(const std::string& titlePart, LinkedList<Film>& result) const
{
    LinkedList<Film> matches;
    for (const Film& film : films_)
    {
        if (film.title().find(titlePart) != std::string::npos)
        {
            matches.pushBack(film);
        }
    }
    result.swap(matches);
}

void CatalogService::rooms(LinkedList<Auditorium>& result) const
{
    LinkedList<Auditorium> matches;
    for (const Auditorium& room : rooms_)
    {
        matches.pushBack(room);
    }
    result.swap(matches);
}

const Film& CatalogService::film(EntityId id) const
{
    if (id <= 0)
    {
        throw std::invalid_argument("ID must be positive");
    }
    const Film* film = films_.findById(id);
    if (film == nullptr)
    {
        throw std::out_of_range("Record not found");
    }
    return *film;
}

const Auditorium& CatalogService::room(EntityId id) const
{
    if (id <= 0)
    {
        throw std::invalid_argument("ID must be positive");
    }
    const Auditorium* room = rooms_.findById(id);
    if (room == nullptr)
    {
        throw std::out_of_range("Record not found");
    }
    return *room;
}

void CatalogService::updateFilm(EntityId id, std::string title, std::int64_t duration)
{
    const Film candidate(id, title, duration);
    const Film& currentFilm = film(id);
    const bool durationChanged = currentFilm.durationMinutes() != duration;
    if (durationChanged && filmReferenced(id))
    {
        throw std::invalid_argument("Cannot change duration of a scheduled film");
    }
    films_.update(id, candidate);
}

void CatalogService::eraseFilm(EntityId id)
{
    film(id);
    if (filmReferenced(id))
    {
        throw std::invalid_argument("Film is referenced by screenings");
    }
    films_.erase(id);
}

void CatalogService::requireRoomUnused(EntityId id) const
{
    room(id);
    for (const Screening& screening : screenings_)
    {
        if (screening.auditoriumId() == id)
        {
            throw std::invalid_argument("Room is referenced by screenings");
        }
    }
}

void CatalogService::prepareSeats(SeatRepository& staged, EntityId roomId,
                                  const Auditorium* replacement) const
{
    // ponytail: rebuild all seats for atomic room edits; use batch updates if data outgrows RAM.
    for (const Seat& seat : seats_)
    {
        if (seat.auditoriumId() != roomId)
        {
            staged.insert(seat);
        }
    }
    if (replacement == nullptr)
    {
        return;
    }
    const EntityId first = seats_.nextId();
    if (replacement->capacity() - 1 > std::numeric_limits<EntityId>::max() - first)
    {
        throw std::overflow_error("Seat IDs exhausted");
    }
    for (std::int64_t offset = 0; offset < replacement->capacity(); ++offset)
    {
        const EntityId seatId = first + offset;
        const std::int64_t row = offset / replacement->seatsPerRow() + 1;
        const std::int64_t column = offset % replacement->seatsPerRow() + 1;
        const Seat seat(seatId, *replacement, row, column);
        staged.insert(seat);
    }
}

void CatalogService::bindSeats(SeatRepository& staged, const Auditorium& room)
{
    for (const Seat& seat : staged)
    {
        if (seat.auditoriumId() == room.id())
        {
            const Seat replacement(seat.id(), room, seat.rowNumber(), seat.seatNumber());
            staged.update(seat.id(), replacement);
        }
    }
}

void CatalogService::prepareTickets(TicketRepository& staged,
                                    const SeatRepository& seats) const
{
    for (const Ticket& ticket : tickets_)
    {
        const Seat* seat = seats.findById(ticket.seatId());
        if (seat == nullptr)
        {
            throw std::invalid_argument("Ticket references a removed seat");
        }
        staged.insert(Ticket(ticket.id(), ticket.screening(), *seat,
                             ticket.soldAt(), ticket.unitPrice()));
    }
}

EntityId CatalogService::addRoom(std::string name, std::int64_t rows, std::int64_t columns)
{
    const EntityId id = rooms_.nextId();
    const Auditorium candidate(id, name, rows, columns);
    SeatRepository staged;
    prepareSeats(staged, id, &candidate);
    TicketRepository stagedTickets;
    prepareTickets(stagedTickets, staged);
    rooms_.insert(candidate);
    bindSeats(staged, *rooms_.findById(id));
    seats_.replaceWith(staged);
    tickets_.relinkWith(stagedTickets);
    return id;
}

void CatalogService::updateRoom(EntityId id, std::string name,
                                std::int64_t rows, std::int64_t columns)
{
    requireRoomUnused(id);
    const Auditorium candidate(id, name, rows, columns);
    SeatRepository staged;
    prepareSeats(staged, id, &candidate);
    TicketRepository stagedTickets;
    prepareTickets(stagedTickets, staged);
    rooms_.update(id, candidate);
    bindSeats(staged, *rooms_.findById(id));
    seats_.replaceWith(staged);
    tickets_.relinkWith(stagedTickets);
}

void CatalogService::eraseRoom(EntityId id)
{
    requireRoomUnused(id);
    SeatRepository staged;
    prepareSeats(staged, id, nullptr);
    TicketRepository stagedTickets;
    prepareTickets(stagedTickets, staged);
    rooms_.erase(id);
    seats_.replaceWith(staged);
    tickets_.relinkWith(stagedTickets);
}

std::unique_ptr<SeatMap> CatalogService::seatMap(EntityId roomId) const
{
    const Auditorium& auditorium = room(roomId);
    return makeSeatMap(auditorium, seats_);
}
