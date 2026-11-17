#pragma once

#include "entities/Auditorium.h"
#include "entities/Film.h"
#include "entities/Screening.h"
#include "entities/Seat.h"
#include "entities/Ticket.h"
#include "persistence/Mapping.h"

namespace txt_persistence {

//  mapping order follows constructors; add argument bindings only if output order must differ.
inline auto filmMapping() {
    return mapping<Film>(
        integerColumn("film_id", &Film::id),
        textColumn("title", &Film::title),
        integerColumn("duration_minutes", &Film::durationMinutes));
}

inline auto auditoriumMapping() {
    return mapping<Auditorium>(
        integerColumn("auditorium_id", &Auditorium::id),
        textColumn("name", &Auditorium::name),
        integerColumn("row_count", &Auditorium::rowCount),
        integerColumn("seats_per_row", &Auditorium::seatsPerRow));
}

inline auto seatMapping(const IRepository<Auditorium>& rooms) {
    return mapping<Seat>(
        integerColumn("seat_id", &Seat::id),
        referenceColumn<Auditorium>("auditorium_id", &Seat::auditoriumId, rooms),
        integerColumn("row_number", &Seat::rowNumber),
        integerColumn("seat_number", &Seat::seatNumber));
}

inline auto screeningMapping(const IRepository<Film>& films, const IRepository<Auditorium>& rooms) {
    return mapping<Screening>(
        integerColumn("screening_id", &Screening::id),
        referenceColumn<Film>("film_id", &Screening::filmId, films),
        referenceColumn<Auditorium>("auditorium_id", &Screening::auditoriumId, rooms),
        textColumn("start_time", &Screening::startTime),
        integerColumn("ticket_price", &Screening::ticketPrice));
}

inline auto ticketMapping(const IRepository<Screening>& screenings, const IRepository<Seat>& seats) {
    return mapping<Ticket>(
        integerColumn("ticket_id", &Ticket::id),
        referenceColumn<Screening>("screening_id", &Ticket::screeningId, screenings),
        referenceColumn<Seat>("seat_id", &Ticket::seatId, seats),
        textColumn("sold_at", &Ticket::soldAt),
        integerColumn("unit_price", &Ticket::unitPrice));
}

}
