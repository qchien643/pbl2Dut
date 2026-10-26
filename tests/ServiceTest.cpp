#ifdef NDEBUG
#undef NDEBUG
#endif

#include "services/CatalogService.h"
#include "services/ScreeningService.h"
#include "services/TicketService.h"

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <new>
#include <sstream>

int allocationsBeforeFailure = -1;

void* operator new(std::size_t size) {
    if (allocationsBeforeFailure == 0) {
        throw std::bad_alloc();
    }
    if (allocationsBeforeFailure > 0) {
        --allocationsBeforeFailure;
    }
    if (void* memory = std::malloc(size == 0 ? 1 : size)) {
        return memory;
    }
    throw std::bad_alloc();
}

void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete[](void* memory) noexcept { ::operator delete(memory); }
void operator delete[](void* memory, std::size_t) noexcept { ::operator delete(memory); }

const char* currentTime = "2026-10-19 10:00";
std::string fakeClock() { return currentTime; }

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

struct Fixture {
    FilmRepository films;
    AuditoriumRepository rooms;
    SeatRepository seats;
    ScreeningRepository screenings;
    TicketRepository tickets;
    CatalogService catalog{films, rooms, seats, screenings};
    ScreeningService schedule{films, rooms, screenings, tickets, fakeClock};
    TicketService sales{films, rooms, seats, screenings, tickets, fakeClock};

    Fixture() {
        currentTime = "2026-10-19 10:00";
        assert(catalog.addFilm("Film", 120) == 1);
        assert(catalog.addRoom("Room", 3, 4) == 1);
        assert(catalog.addRoom("Other room", 1, 2) == 2);
    }

    void markSaved() {
        films.markSaved();
        rooms.markSaved();
        seats.markSaved();
        screenings.markSaved();
        tickets.markSaved();
    }
};

void testCatalog() { // FR-FILM-*, FR-ROOM-*, SAT-01, SAT-13.
    Fixture f;
    assert(f.seats.size() == 14);
    auto map = f.catalog.seatMap(1);
    assert(map->rows() == 3 && map->columns() == 4);
    for (std::size_t row = 0; row < 3; ++row) {
        for (std::size_t column = 0; column < 4; ++column) {
            const auto& cell = map->at(row, column);
            const auto* seat = f.seats.findById(cell.seatId);
            assert(seat && seat->rowNumber() == static_cast<std::int64_t>(row + 1));
            assert(seat->seatNumber() == static_cast<std::int64_t>(column + 1) && !cell.sold);
        }
    }
    f.markSaved();
    expectThrow([&] { f.catalog.addRoom("", 3, 4); });
    expectThrow([&] { f.catalog.addRoom("Invalid", 0, 4); });
    expectThrow<std::overflow_error>([&] {
        f.catalog.addRoom("Too large", std::numeric_limits<std::int64_t>::max(), 2);
    });
    expectThrow([&] { f.catalog.addFilm(" ", 10); });
    expectThrow([&] { f.catalog.updateFilm(1, "Film", -1); });
    expectThrow<std::out_of_range>([&] { f.catalog.eraseFilm(999); });
    expectThrow<std::out_of_range>([&] { f.catalog.updateRoom(999, "X", 1, 1); });
    expectThrow([&] { f.catalog.room(0); });
    assert(!f.rooms.isDirty() && !f.seats.isDirty() && !f.films.isDirty());

    f.catalog.updateRoom(2, "Resized", 2, 2);
    assert(f.seats.size() == 16 && f.catalog.room(2).capacity() == 4);
    assert(!f.seats.findById(13) && !f.seats.findById(14));
    assert(f.catalog.seatMap(2)->at(1, 1).seatId == 18);
    assert(f.catalog.seatMap(1)->at(0, 0).seatId == 1);
    f.catalog.eraseRoom(2);
    assert(f.seats.size() == 12 && f.seats.nextId() == 19);
    assert(f.catalog.addRoom("New room", 1, 1) == 3);
    assert(f.catalog.seatMap(3)->at(0, 0).seatId == 19);
    f.catalog.updateFilm(1, "Renamed", 90);
    assert(f.catalog.film(1).durationMinutes() == 90);
    const auto otherFilm = f.catalog.addFilm("Another", 80);
    LinkedList<Film> films;
    f.catalog.films("name", films);
    assert(films.size() == 1 && (*films.begin()).id() == 1);
    f.catalog.films("", films);
    assert(films.size() == 2);
    f.catalog.films("Not found", films);
    assert(films.empty());
    LinkedList<Auditorium> rooms;
    f.catalog.rooms(rooms);
    assert(rooms.size() == 2);
    f.catalog.rooms(rooms);
    assert(rooms.size() == 2);
    f.catalog.eraseFilm(otherFilm);
    f.schedule.add(1, 1, "2026-11-01 18:00", 80000);
    f.markSaved();
    expectThrow([&] { f.catalog.eraseFilm(1); });
    expectThrow([&] { f.catalog.updateFilm(1, "Changed", 100); });
    expectThrow([&] { f.catalog.updateRoom(1, "Changed", 3, 4); });
    expectThrow([&] { f.catalog.eraseRoom(1); });
    assert(!f.films.isDirty() && !f.rooms.isDirty() && !f.seats.isDirty());
    f.catalog.updateFilm(1, "Allowed rename", 90);
    assert(f.catalog.film(1).title() == "Allowed rename");
}

void testScheduling() { // FR-SHOW-*, SAT-02, SAT-03.
    Fixture f;
    const auto first = f.schedule.add(1, 1, "2026-11-01 23:00", 80000);
    assert(f.schedule.endTime(first) == "2026-11-02 01:00");
    f.markSaved();
    for (const char* start : {"2026-11-01 23:00", "2026-11-01 22:00",
                              "2026-11-01 20:46", "2026-11-02 01:14"}) {
        expectThrow([&] { f.schedule.add(1, 1, start, 80000); });
    }
    expectThrow([&] { f.schedule.add(1, 1, currentTime, 80000); });
    expectThrow([&] { f.schedule.add(1, 1, "2026-10-19 09:59", 80000); });
    expectThrow<std::out_of_range>([&] { f.schedule.add(999, 1, "2026-11-02 18:00", 80000); });
    expectThrow<std::out_of_range>([&] { f.schedule.add(1, 999, "2026-11-02 18:00", 80000); });
    expectThrow([&] { f.schedule.add(1, 1, "2026-11-31 18:00", 80000); });
    expectThrow([&] { f.schedule.add(1, 1, "2026-11-02 18:00", 0); });
    assert(!f.screenings.isDirty() && f.screenings.size() == 1 && f.screenings.nextId() == 2);
    const auto after = f.schedule.add(1, 1, "2026-11-02 01:15", 90000);
    f.schedule.add(1, 1, "2026-11-01 20:45", 80000);
    const auto otherRoom = f.schedule.add(1, 2, "2026-11-01 23:00", 80000);
    f.schedule.update(first, 1, 1, "2026-11-01 23:00", 85000);
    assert(f.schedule.find(first).ticketPrice() == 85000);
    f.markSaved();
    expectThrow([&] { f.schedule.update(after, 1, 1, "2026-11-02 01:14", 90000); });
    assert(!f.screenings.isDirty() && f.schedule.find(after).startTime() == "2026-11-02 01:15");
    f.schedule.erase(otherRoom);
    assert(!f.screenings.findById(otherRoom));
    LinkedList<Screening> screenings;
    f.schedule.list("2026-11-02", 1, 1, true, screenings);
    assert(screenings.size() == 1 && (*screenings.begin()).id() == after);
    currentTime = "2026-11-02 01:15";
    f.schedule.list("", 0, 0, true, screenings);
    assert(screenings.empty());
    f.schedule.list("", 0, 0, false, screenings);
    assert(screenings.size() == 3);
    expectThrow([&] { f.schedule.list("2026-02-30", 0, 0, false, screenings); });
    expectThrow([&] { f.schedule.list("", -1, 0, false, screenings); });
    assert(screenings.size() == 3);
    expectThrow<std::out_of_range>([&] { f.schedule.erase(999); });
}

void testLookupValidation() {
    Fixture f;
    const EntityId show = f.schedule.add(1, 1, "2026-11-01 18:00", 80000);
    for (EntityId id : {EntityId(0), EntityId(-1)}) {
        expectThrow([&] { f.catalog.film(id); });
        expectThrow([&] { f.catalog.room(id); });
        expectThrow([&] { f.schedule.find(id); });
        expectThrow([&] { f.sales.find(id); });
        expectThrow([&] { f.sales.preview(id, 1); });
        expectThrow([&] { f.sales.preview(show, id); });
        expectThrow([&] { f.sales.seatMap(id); });
    }
    expectThrow<std::out_of_range>([&] { f.catalog.film(999); });
    expectThrow<std::out_of_range>([&] { f.catalog.room(999); });
    expectThrow<std::out_of_range>([&] { f.schedule.find(999); });
    expectThrow<std::out_of_range>([&] { f.sales.seatMap(999); });
    assert(&f.catalog.film(1) == f.films.findById(1));
    assert(&f.catalog.room(1) == f.rooms.findById(1));
    assert(&f.schedule.find(show) == f.screenings.findById(show));

    f.films.erase(1);
    expectThrow<std::out_of_range>([&] { f.sales.preview(show, 1); });
    expectThrow<std::out_of_range>([&] { f.schedule.endTime(show); });
    f.rooms.erase(1);
    expectThrow<std::out_of_range>([&] { f.sales.preview(show, 1); });
    expectThrow<std::out_of_range>([&] { f.sales.seatMap(show); });
    assert(f.tickets.empty());
}

void testQueryResultOwnership() {
    Fixture f;
    f.markSaved();
    LinkedList<Film> films;
    f.catalog.films("", films);
    assert(&*films.begin() != f.films.findById(1));
    f.catalog.updateFilm(1, "Renamed", 120);
    assert((*films.begin()).title() == "Film");
    films.clear();
    assert(f.films.size() == 1);
    films.pushBack(Film(99, "Keep on failure", 90));
    f.markSaved();

    allocationsBeforeFailure = 0;
    bool failed = false;
    try {
        f.catalog.films("", films);
    } catch (const std::bad_alloc&) {
        failed = true;
    }
    allocationsBeforeFailure = -1;
    assert(failed && films.size() == 1 && (*films.begin()).id() == 99);
    assert(!f.films.isDirty());
    f.catalog.films("", films);
    assert(films.size() == 1 && (*films.begin()).title() == "Renamed");
    assert(!f.films.isDirty());
}

void testCalendarAndOverflow() {
    Fixture f;
    currentTime = "0001-01-01 00:00";
    const char* starts[] = {"0001-01-01 00:01", "1900-02-28 23:00", "2000-02-28 23:00",
                            "2000-02-29 23:00", "2026-12-31 23:00", "9999-12-31 21:44"};
    const char* ends[] = {"0001-01-01 02:01", "1900-03-01 01:00", "2000-02-29 01:00",
                          "2000-03-01 01:00", "2027-01-01 01:00", "9999-12-31 23:44"};
    for (std::size_t i = 0; i < 6; ++i) {
        const auto id = f.schedule.add(1, 1, starts[i], 1);
        assert(f.schedule.endTime(id) == ends[i]);
    }
    expectThrow<std::overflow_error>([&] { f.schedule.add(1, 2, "9999-12-31 21:45", 1); });
    const auto huge = f.catalog.addFilm("Huge", std::numeric_limits<std::int64_t>::max());
    expectThrow<std::overflow_error>([&] { f.schedule.add(huge, 2, "2027-01-01 00:00", 1); });
    f.seats.insert(Seat(std::numeric_limits<EntityId>::max(), 2, 1, 3));
    const auto size = f.seats.size();
    expectThrow<std::overflow_error>([&] { f.catalog.updateRoom(2, "No ID space", 1, 1); });
    assert(f.seats.size() == size && f.catalog.room(2).capacity() == 2);
    f.catalog.eraseRoom(2);
    expectThrow<std::overflow_error>([&] { f.seats.nextId(); });
    expectThrow([&] {
        ScreeningService invalid(f.films, f.rooms, f.screenings, f.tickets, nullptr);
    });
    expectThrow([&] {
        TicketService invalid(f.films, f.rooms, f.seats, f.screenings, f.tickets, nullptr);
    });
    assert(entity_detail::dateTime(localMinuteNow()).size() == 16);
}

void testSales() { // SAT-04/05/06/07/10/11/13, FR-SALE-*, FR-QUERY-*, FR-REPORT-*.
    Fixture f;
    const auto first = f.schedule.add(1, 1, "2026-11-01 18:00", 80000);
    const auto second = f.schedule.add(1, 1, "2026-11-01 20:15", 90000);
    f.markSaved();
    const auto preview = f.sales.preview(first, 1);
    assert(preview.film.title() == "Film" && preview.room.id() == 1);
    assert(preview.endTime == "2026-11-01 20:00" && preview.screening.ticketPrice() == 80000);
    expectThrow([&] { f.sales.sell(first, 1, false); });
    assert(f.tickets.empty() && !f.tickets.isDirty() && !f.sales.seatMap(first)->at(0, 0).sold);
    expectThrow([&] { f.sales.sell(first, 13, true); });
    expectThrow<std::out_of_range>([&] { f.sales.sell(first, 999, true); });
    expectThrow<std::out_of_range>([&] { f.sales.sell(999, 1, true); });
    assert(f.tickets.empty());
    const auto ticket = f.sales.sell(first, 1, true);
    assert(ticket == 1 && f.tickets.isDirty());
    assert(f.sales.seatMap(first)->at(0, 0).sold && !f.sales.seatMap(second)->at(0, 0).sold);
    f.markSaved();
    expectThrow([&] { f.sales.sell(first, 1, true); });
    assert(f.tickets.size() == 1 && !f.tickets.isDirty() && f.tickets.nextId() == 2);
    assert(f.sales.sell(second, 1, true) == 2);
    auto report = f.sales.revenue("2026-10-19");
    assert(report.ticketCount == 2 && report.amount == 170000);
    int listed = 0;
    LinkedList<TicketDetails> tickets;
    f.sales.listBySaleDate("2026-10-19", tickets);
    for (const TicketDetails& details : tickets) {
        assert(details.ticket.id() == listed + 1);
        assert(details.sale.film.title() == "Film" && details.sale.room.name() == "Room");
        ++listed;
    }
    assert(listed == 2);
    report = f.sales.revenue("2026-11-01");
    assert(report.ticketCount == 0 && report.amount == 0);
    f.sales.listBySaleDate("2026-11-01", tickets);
    assert(tickets.empty());
    const auto found = f.sales.find(ticket);
    assert(found.ticket.unitPrice() == 80000 && found.ticket.soldAt() == "2026-10-19 10:00");
    assert(found.sale.seat.rowNumber() == 1 && found.sale.seat.seatNumber() == 1);
    expectThrow([&] { f.sales.find(0); });
    expectThrow<std::out_of_range>([&] { f.sales.find(999); });
    expectThrow([&] { f.sales.revenue("2026-02-30"); });
    expectThrow([&] { f.sales.listBySaleDate("2026-02-30", tickets); });
    f.markSaved();
    expectThrow([&] { f.schedule.erase(first); });
    expectThrow([&] { f.schedule.update(first, 1, 1, "2026-11-02 18:00", 1); });
    assert(!f.screenings.isDirty());
    f.catalog.updateFilm(1, "Current title", 120);
    assert(f.sales.find(ticket).sale.film.title() == "Current title");

    currentTime = "2026-11-01 17:59";
    f.sales.preview(first, 2);
    currentTime = "2026-11-01 18:00";
    expectThrow([&] { f.sales.sell(first, 2, true); });
    currentTime = "2026-11-01 18:01";
    expectThrow([&] { f.sales.sell(first, 2, true); });
    assert(f.tickets.size() == 2 && !f.tickets.isDirty());
    assert(f.sales.find(ticket).ticket.id() == ticket);
    currentTime = "bad clock";
    expectThrow([&] { f.sales.sell(second, 2, true); });

    // Report uses ticket snapshots, not the current screening price.
    f.screenings.update(first, Screening(first, 1, 1, "2026-11-01 18:00", 1));
    assert(f.sales.revenue("2026-10-19").amount == 170000);
    f.tickets.insert(Ticket(3, first, 2, "2026-10-19 11:00", std::numeric_limits<std::int64_t>::max()));
    expectThrow<std::overflow_error>([&] { f.sales.revenue("2026-10-19"); });
}

void testAllocationRollback() { // FR-ROOM-07/08, FR-SALE-09, NFR-03/11.
    for (int operation = 0; operation < 6; ++operation) {
        for (bool initiallyDirty : {false, true}) {
            bool succeeded = false;
            for (int budget = 0; budget < 500; ++budget) {
                Fixture f;
                f.schedule.add(1, 1, "2026-11-01 18:00", 80000);
                for (EntityId seat = 1; seat <= 8; ++seat) {
                    f.sales.sell(1, seat, true);
                }
                const auto spare = f.schedule.add(1, 1, "2026-11-02 18:00", 80000);
                if (!initiallyDirty) {
                    f.markSaved();
                }
                const Seat* originalSeat = f.seats.findById(13);
                allocationsBeforeFailure = budget;
                bool failed = false;
                try {
                    switch (operation) {
                    case 0: f.catalog.addRoom("New room with a title requiring allocation", 2, 5); break;
                    case 1: f.catalog.updateRoom(2, "New room title requiring allocation", 2, 5); break;
                    case 2: f.catalog.eraseRoom(2); break;
                    case 3: f.sales.sell(1, 9, true); break;
                    case 4: f.catalog.updateFilm(1, "New film title requiring allocation", 120); break;
                    case 5: f.schedule.update(spare, 1, 1, "2026-11-03 18:00", 90000); break;
                    }
                } catch (const std::bad_alloc&) {
                    failed = true;
                }
                allocationsBeforeFailure = -1;
                if (failed) {
                    assert(f.rooms.size() == 2 && f.rooms.nextId() == 3);
                    assert(f.catalog.room(2).name() == "Other room" && f.catalog.room(2).capacity() == 2);
                    assert(f.seats.size() == 14 && f.seats.nextId() == 15);
                    assert(f.seats.findById(13) == originalSeat);
                    assert(f.tickets.size() == 8 && f.tickets.nextId() == 9);
                    assert(!f.sales.seatMap(1)->at(2, 0).sold);
                    assert(f.sales.revenue("2026-10-19").amount == 640000);
                    assert(f.catalog.film(1).title() == "Film");
                    assert(f.schedule.find(spare).startTime() == "2026-11-02 18:00");
                    assert(f.rooms.isDirty() == initiallyDirty && f.seats.isDirty() == initiallyDirty);
                    assert(f.tickets.isDirty() == initiallyDirty && f.films.isDirty() == initiallyDirty);
                    assert(f.screenings.isDirty() == initiallyDirty);
                } else {
                    succeeded = true;
                    if (operation <= 2) {
                        assert(f.rooms.isDirty() && f.seats.isDirty());
                        assert(f.seats.size() == (operation == 0 ? 24u : operation == 1 ? 22u : 12u));
                        if (operation != 2) {
                            assert(f.catalog.seatMap(operation == 0 ? 3 : 2)->size() == 10);
                        }
                    } else if (operation == 3) {
                        assert(f.tickets.size() == 9 && f.sales.seatMap(1)->at(2, 0).sold);
                    } else if (operation == 4) {
                        assert(f.catalog.film(1).title() == "New film title requiring allocation");
                    } else {
                        assert(f.schedule.find(spare).ticketPrice() == 90000);
                    }
                }
                for (const Seat& seat : f.seats) {
                    assert(f.seats.findById(seat.id()) == &seat);
                }
                if (succeeded) {
                    break;
                }
            }
            assert(succeeded);
        }
    }
}

void testInvalidMapsAndReconfirmation() {
    Fixture f;
    const auto show = f.schedule.add(1, 1, "2026-11-01 18:00", 80000);
    assert(f.sales.preview(show, 1).screening.ticketPrice() == 80000);
    assert(f.sales.preview(show, 1).screening.ticketPrice() == 80000);
    f.schedule.update(show, 1, 1, "2026-11-01 18:00", 90000);
    const auto ticket = f.sales.sell(show, 1, true);
    assert(f.sales.find(ticket).ticket.unitPrice() == 90000);
    expectThrow([&] { f.sales.sell(show, 1, true); });

    f.seats.erase(2);
    expectThrow([&] { f.catalog.seatMap(1); });
    f.seats.insert(Seat(2, 1, 1, 2));
    f.seats.insert(Seat(15, 1, 1, 2));
    expectThrow([&] { f.sales.seatMap(show); });
    f.seats.erase(15);
    f.seats.insert(Seat(16, 1, 4, 1));
    expectThrow([&] { f.sales.seatMap(show); });
    expectThrow([&] { f.sales.sell(show, 16, true); });
    f.seats.erase(16);
    f.tickets.insert(Ticket(2, show, 1, "2026-10-19 10:00", 90000));
    expectThrow([&] { f.sales.seatMap(show); });

    Fixture other;
    other.schedule.add(1, 1, "2026-11-01 18:00", 80000);
    other.tickets.insert(Ticket(1, 1, 13, "2026-10-19 10:00", 80000));
    expectThrow([&] { other.sales.seatMap(1); });
    expectThrow([&] { other.sales.find(1); });
}

void testPerformanceBaseline() { // NFR-02.
    FilmRepository films;
    AuditoriumRepository rooms;
    SeatRepository seats;
    ScreeningRepository screenings;
    TicketRepository tickets;
    CatalogService catalog(films, rooms, seats, screenings);
    for (int index = 1; index <= 100; ++index) {
        catalog.addFilm("Film " + std::to_string(index), 90);
    }
    for (int index = 1; index <= 10; ++index) {
        catalog.addRoom("Room " + std::to_string(index), 10, 20);
    }
    EntityId ticketId = 1;
    for (int index = 0; index < 1000; ++index) {
        const EntityId screeningId = index + 1;
        const EntityId filmId = index % 100 + 1;
        const EntityId roomId = index % 10 + 1;
        const int dayIndex = index / 10;
        std::ostringstream start;
        start << "2099-" << std::setfill('0') << std::setw(2) << dayIndex / 28 + 1
              << '-' << std::setw(2) << dayIndex % 28 + 1 << " 18:00";
        assert(screenings.insert(Screening(screeningId, filmId, roomId, start.str(), 80000)));
        const EntityId firstSeat = (roomId - 1) * 200 + 1;
        for (int seat = 0; seat < 10; ++seat) {
            assert(tickets.insert(Ticket(ticketId++, screeningId, firstSeat + seat,
                                         "2026-10-19 10:00", 80000)));
        }
    }
    ScreeningService schedule(films, rooms, screenings, tickets, fakeClock);
    TicketService sales(films, rooms, seats, screenings, tickets, fakeClock);
    const auto check = [](auto operation) {
        const auto start = std::chrono::steady_clock::now();
        operation();
        assert(std::chrono::steady_clock::now() - start < std::chrono::seconds(2));
    };
    check([&] {
        LinkedList<Screening> matches;
        schedule.list("2099-01-01", 0, 0, false, matches);
        assert(matches.size() == 10);
    });
    check([&] { assert(sales.find(10000).ticket.id() == 10000); });
    check([&] {
        auto map = sales.seatMap(1);
        assert(map->size() == 200 && map->at(0, 0).sold && !map->at(0, 10).sold);
    });
    check([&] {
        const auto revenue = sales.revenue("2026-10-19");
        assert(revenue.ticketCount == 10000 && revenue.amount == 800000000);
    });
}

int main() {
    testCatalog();
    testScheduling();
    testLookupValidation();
    testQueryResultOwnership();
    testCalendarAndOverflow();
    testSales();
    testAllocationRollback();
    testInvalidMapsAndReconfirmation();
    testPerformanceBaseline();
    std::cout << "Service checks passed\n";
}
