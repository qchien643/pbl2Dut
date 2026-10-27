#pragma once

#include "repositories/AuditoriumRepository.h"
#include "repositories/FilmRepository.h"
#include "repositories/ScreeningRepository.h"
#include "repositories/SeatRepository.h"
#include "repositories/TicketRepository.h"
#include "services/SeatMap.h"

#include <cstdint>
#include <memory>
#include <string>

class CatalogService
{
public:
    CatalogService(FilmRepository& films, AuditoriumRepository& rooms,
                   SeatRepository& seats, const ScreeningRepository& screenings,
                   TicketRepository& tickets);

    EntityId addFilm(std::string title, std::int64_t duration);
    void updateFilm(EntityId id, std::string title, std::int64_t duration);
    void eraseFilm(EntityId id);
    const Film& film(EntityId id) const;

    // Query results are copies; replace the output only after a successful query.
    void films(const std::string& titlePart, LinkedList<Film>& result) const;

    EntityId addRoom(std::string name, std::int64_t rows, std::int64_t columns);
    void updateRoom(EntityId id, std::string name, std::int64_t rows, std::int64_t columns);
    void eraseRoom(EntityId id);
    const Auditorium& room(EntityId id) const;

    void rooms(LinkedList<Auditorium>& result) const;

    std::unique_ptr<SeatMap> seatMap(EntityId roomId) const;

private:
    bool filmReferenced(EntityId id) const;
    void requireRoomUnused(EntityId id) const;
    void prepareSeats(SeatRepository& staged, EntityId roomId,
                      const Auditorium* replacement) const;
    static void bindSeats(SeatRepository& staged, const Auditorium& room);

    FilmRepository& films_;
    AuditoriumRepository& rooms_;
    SeatRepository& seats_;
    const ScreeningRepository& screenings_;
    TicketRepository& tickets_;
};
