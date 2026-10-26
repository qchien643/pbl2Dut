#ifdef NDEBUG
#undef NDEBUG
#endif

#include "entities/Auditorium.h"
#include "entities/Film.h"
#include "entities/Screening.h"
#include "entities/Seat.h"
#include "entities/Ticket.h"
#include "entities/EntityValidation.h"

#include <cassert>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

template <typename Exception = std::invalid_argument, typename Function>
void expectThrow(Function function) {
    bool threw = false;
    try {
        function();
    } catch (const Exception&) {
        threw = true;
    }
    assert(threw);
}

void testValidEntities() {
    const Film film(1, "  Film one  ", 120);
    assert(film.id() == 1 && film.title() == "Film one" && film.durationMinutes() == 120);
    const Film vietnamese(2, u8"Phim Vi\u1ec7t", 90);
    assert(vietnamese.title() == u8"Phim Vi\u1ec7t");
    const Auditorium room(3, " Room 3 ", 6, 8);
    assert(room.id() == 3 && room.name() == "Room 3");
    assert(room.rowCount() == 6 && room.seatsPerRow() == 8 && room.capacity() == 48);
    const Seat seat(11, 3, 2, 3);
    assert(seat.id() == 11 && seat.auditoriumId() == 3);
    assert(seat.rowNumber() == 2 && seat.seatNumber() == 3);
    const Screening screening(4, 1, 3, "2026-11-01 18:00", 80000);
    assert(screening.id() == 4 && screening.filmId() == 1 && screening.auditoriumId() == 3);
    assert(screening.startTime() == "2026-11-01 18:00" && screening.ticketPrice() == 80000);
    const Ticket ticket(5, 4, 11, "2026-10-31 10:00", 80000);
    assert(ticket.id() == 5 && ticket.screeningId() == 4 && ticket.seatId() == 11);
    assert(ticket.soldAt() == "2026-10-31 10:00" && ticket.unitPrice() == 80000);
    assert(Film(1, "Same title", 90).title() == Film(2, "Same title", 90).title());
    const auto maximum = std::numeric_limits<std::int64_t>::max();
    assert(Auditorium(1, "Maximum", 1, maximum).capacity() == maximum);
    assert(Ticket(maximum, 1, 1, "2026-11-01 00:00", maximum).unitPrice() == maximum);
}

void testInvalidValues() {
    expectThrow([] { Film invalid(0, "Film", 120); });
    expectThrow([] { Film invalid(-1, "Film", 120); });
    expectThrow([] { Film invalid(1, "Film", 0); });
    expectThrow([] { Film invalid(1, "Film", -1); });
    const std::string invalidNames[] = {"", "   ", "Film|one", "Film\none", "Film\rone", "Film\tone", std::string("Film\0one", 8), std::string(1, '\x7f')};
    for (const std::string& name : invalidNames) {
        expectThrow([&] { Film invalid(1, name, 120); });
        expectThrow([&] { Auditorium invalid(1, name, 1, 1); });
    }
    expectThrow([] { Auditorium invalid(0, "Room", 1, 1); });
    expectThrow([] { Auditorium invalid(1, "Room", 0, 1); });
    expectThrow([] { Auditorium invalid(1, "Room", 1, -1); });
    expectThrow<std::overflow_error>([] {
        Auditorium invalid(1, "Room", std::numeric_limits<std::int64_t>::max(), 2);
    });
    expectThrow([] { Seat invalid(0, 1, 1, 1); });
    expectThrow([] { Seat invalid(1, 0, 1, 1); });
    expectThrow([] { Seat invalid(1, 1, 0, 1); });
    expectThrow([] { Seat invalid(1, 1, 1, -1); });
    expectThrow([] { Screening invalid(0, 1, 1, "2026-11-01 18:00", 80000); });
    expectThrow([] { Screening invalid(1, 0, 1, "2026-11-01 18:00", 80000); });
    expectThrow([] { Screening invalid(1, 1, 0, "2026-11-01 18:00", 80000); });
    expectThrow([] { Screening invalid(1, 1, 1, "2026-11-01 18:00", 0); });
    expectThrow([] { Ticket invalid(0, 1, 1, "2026-11-01 10:00", 80000); });
    expectThrow([] { Ticket invalid(1, 0, 1, "2026-11-01 10:00", 80000); });
    expectThrow([] { Ticket invalid(1, 1, 0, "2026-11-01 10:00", 80000); });
    expectThrow([] { Ticket invalid(1, 1, 1, "2026-11-01 10:00", -1); });
}

void testDateTime() {
    const char* validDates[] = {"0001-01-01 00:00", "2000-02-29 23:59", "2024-02-29 12:30", "2026-04-30 00:00", "9999-12-31 23:59"};
    for (const char* date : validDates) {
        assert(Screening(1, 1, 1, date, 1).startTime() == date);
        assert(Ticket(1, 1, 1, date, 1).soldAt() == date);
    }
    const char* invalidDates[] = {
        "", "2026-1-01 10:00", "2026-01-01", "2026-01-01 10:00 ",
        "2026-01-01T10:00", "2026/01/01 10:00", "2026-01-01 10:0x",
        "0000-01-01 10:00", "2026-00-01 10:00", "2026-13-01 10:00",
        "2026-01-00 10:00", "2026-01-32 10:00", "2026-04-31 10:00",
        "1900-02-29 10:00", "2100-02-29 10:00", "2026-02-29 10:00",
        "2024-02-30 10:00", "2026-01-01 24:00", "2026-01-01 23:60"
    };
    for (const char* date : invalidDates) {
        expectThrow([&] { Screening invalid(1, 1, 1, date, 1); });
        expectThrow([&] { Ticket invalid(1, 1, 1, date, 1); });
    }
}

void testValidationHelpers()
{
    assert(entity_detail::positive(1, "ID") == 1);
    expectThrow([] { entity_detail::positive(0, "ID"); });
    expectThrow([] { entity_detail::positive(-1, "ID"); });

    const std::string original = "  Film name  ";
    assert(entity_detail::name(original) == "Film name");
    assert(original == "  Film name  ");
    assert(entity_detail::name("A") == "A");
    expectThrow([] { entity_detail::name("  |  "); });

    assert(entity_detail::dateTime("2400-02-29 00:00") == "2400-02-29 00:00");
    expectThrow([] { entity_detail::dateTime("2200-02-29 00:00"); });
    expectThrow([] { entity_detail::dateTime("2026-01-01 0x:00"); });
    expectThrow([] { entity_detail::dateTime("2026-01-01 00:0x"); });
}

int main() {
    testValidEntities();
    testInvalidValues();
    testDateTime();
    testValidationHelpers();
    std::cout << "Entity checks passed\n";
}
