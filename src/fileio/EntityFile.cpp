#include "fileio/EntityFile.h"
#include "helper/FieldValidation.h"

#include <stdexcept>

namespace fileio {

const char* const filmHeader = "film_id|title|duration_minutes";
const char* const auditoriumHeader = "auditorium_id|name|row_count|seats_per_row";
const char* const seatHeader = "seat_id|auditorium_id|row_number|seat_number";
const char* const screeningHeader = "screening_id|film_id|auditorium_id|start_time|ticket_price";
const char* const ticketHeader = "ticket_id|screening_id|seat_id|sold_at|unit_price";

Film readFilm(const Row& row) {
    const EntityId id = number(row.at("film_id"));
    const std::string title = row.at("title");
    const std::int64_t duration = number(row.at("duration_minutes"));
    return Film(id, title, duration);
}

Auditorium readAuditorium(const Row& row) {
    const EntityId id = number(row.at("auditorium_id"));
    const std::string name = row.at("name");
    const std::int64_t rows = number(row.at("row_count"));
    const std::int64_t columns = number(row.at("seats_per_row"));
    return Auditorium(id, name, rows, columns);
}

Seat readSeat(const Row& row, const IRepository<Auditorium>& rooms) {
    const EntityId id = number(row.at("seat_id"));
    const EntityId roomId = number(row.at("auditorium_id"));
    field_validation::requirePositiveNumber(roomId, "auditorium_id");
    const Auditorium* room = rooms.findById(roomId);
    if (room == nullptr) {
        throw std::invalid_argument("Seat references a missing room");
    }
    const std::int64_t rowNumber = number(row.at("row_number"));
    const std::int64_t seatNumber = number(row.at("seat_number"));
    return Seat(id, *room, rowNumber, seatNumber);
}

Screening readScreening(const Row& row, const IRepository<Film>& films,
                        const IRepository<Auditorium>& rooms) {
    const EntityId id = number(row.at("screening_id"));
    const EntityId filmId = number(row.at("film_id"));
    const EntityId roomId = number(row.at("auditorium_id"));
    field_validation::requirePositiveNumber(filmId, "film_id");
    field_validation::requirePositiveNumber(roomId, "auditorium_id");
    const Film* film = films.findById(filmId);
    const Auditorium* room = rooms.findById(roomId);
    if (film == nullptr || room == nullptr) {
        throw std::invalid_argument("Screening references a missing film or room");
    }
    const std::string start = row.at("start_time");
    const std::int64_t price = number(row.at("ticket_price"));
    return Screening(id, *film, *room, start, price);
}

Ticket readTicket(const Row& row, const IRepository<Screening>& screenings,
                  const IRepository<Seat>& seats) {
    const EntityId id = number(row.at("ticket_id"));
    const EntityId screeningId = number(row.at("screening_id"));
    const EntityId seatId = number(row.at("seat_id"));
    field_validation::requirePositiveNumber(screeningId, "screening_id");
    field_validation::requirePositiveNumber(seatId, "seat_id");
    const Screening* screening = screenings.findById(screeningId);
    const Seat* seat = seats.findById(seatId);
    if (screening == nullptr || seat == nullptr) {
        throw std::invalid_argument("Ticket references a missing screening or seat");
    }
    const std::string soldAt = row.at("sold_at");
    const std::int64_t price = number(row.at("unit_price"));
    return Ticket(id, *screening, *seat, soldAt, price);
}

void writeFilm(std::ostream& output, const Film& film) {
    validateUtf8(film.title());
    output << film.id() << '|' << film.title() << '|' << film.durationMinutes() << '\n';
}

void writeAuditorium(std::ostream& output, const Auditorium& room) {
    validateUtf8(room.name());
    output << room.id() << '|' << room.name() << '|'
           << room.rowCount() << '|' << room.seatsPerRow() << '\n';
}

void writeSeat(std::ostream& output, const Seat& seat) {
    output << seat.id() << '|' << seat.auditoriumId() << '|'
           << seat.rowNumber() << '|' << seat.seatNumber() << '\n';
}

void writeScreening(std::ostream& output, const Screening& screening) {
    output << screening.id() << '|' << screening.filmId() << '|'
           << screening.auditoriumId() << '|' << screening.startTime() << '|'
           << screening.ticketPrice() << '\n';
}

void writeTicket(std::ostream& output, const Ticket& ticket) {
    output << ticket.id() << '|' << ticket.screeningId() << '|' << ticket.seatId() << '|'
           << ticket.soldAt() << '|' << ticket.unitPrice() << '\n';
}

}
