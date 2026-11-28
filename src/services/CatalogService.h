#pragma once

#include "entities/Auditorium.h"
#include "entities/Film.h"
#include "entities/Screening.h"
#include "entities/Seat.h"
#include "repositories/IRepository.h"
#include "repositories/IFilmRepository.h"
#include "repositories/IRoomStorage.h"
#include "helper/SeatMap.h"

#include <cstdint>
#include <memory>
#include <string>

class CatalogService
{
public:
    CatalogService(IFilmRepository& films,
                   IRepository<Auditorium>& rooms,
                   IRepository<Seat>& seats, const IRepository<Screening>& screenings,
                   IRoomStorage& roomStorage);

    EntityId addFilm(std::string title, std::int64_t duration);
    void updateFilm(EntityId id, std::string title, std::int64_t duration);
    // Reject deletion while any screening references this film, including history.
    void eraseFilm(EntityId id);
    const Film& film(EntityId id) const;

    // Query results are copies; replace the output only after a successful query.
    void films(const std::string& titlePart, LinkedList<Film>& result) const;

    EntityId addRoom(std::string name, std::int64_t rows, std::int64_t columns);
    void updateRoom(EntityId id, std::string name, std::int64_t rows, std::int64_t columns);
    // Reject rooms with screenings; otherwise delete their seats before the room.
    void eraseRoom(EntityId id);
    const Auditorium& room(EntityId id) const;

    void rooms(LinkedList<Auditorium>& result) const;

    std::unique_ptr<SeatMap> seatMap(EntityId roomId) const;
    // Shared room-layout rule; TicketService adds screening-specific sold status.
    static std::unique_ptr<SeatMap> makeRoomSeatMap(const Auditorium& room,
                                                  const IRepository<Seat>& seats);

private:
    void requireUniqueFilmTitle(const Film& candidate) const;
    void requireUniqueRoomName(const Auditorium& candidate) const;
    bool filmReferenced(EntityId id) const;
    void requireRoomUnused(EntityId id) const;
    void prepareSeats(LinkedList<Seat>& staged, const Auditorium& replacement) const;

    IFilmRepository& films_;
    IRepository<Auditorium>& rooms_;
    IRepository<Seat>& seats_;
    const IRepository<Screening>& screenings_;
    IRoomStorage& roomStorage_;
};
