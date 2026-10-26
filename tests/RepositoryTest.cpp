#ifdef NDEBUG
#undef NDEBUG
#endif

#include "repositories/AuditoriumRepository.h"
#include "repositories/FilmRepository.h"
#include "repositories/ScreeningRepository.h"
#include "repositories/SeatRepository.h"
#include "repositories/TicketRepository.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

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

void operator delete(void* memory) noexcept {
    std::free(memory);
}

void operator delete(void* memory, std::size_t) noexcept {
    std::free(memory);
}

void* operator new[](std::size_t size) {
    return ::operator new(size);
}

void operator delete[](void* memory) noexcept {
    ::operator delete(memory);
}

void operator delete[](void* memory, std::size_t) noexcept {
    ::operator delete(memory);
}

template <typename Exception, typename Function>
void expectThrow(Function function) {
    bool threw = false;
    try {
        function();
    } catch (const Exception&) {
        threw = true;
    }
    assert(threw);
}

void testCrudAndIds() {
    FilmRepository films;
    assert(films.empty() && films.size() == 0 && films.nextId() == 1);
    assert(!films.isDirty() && films.findById(1) == nullptr && !films.erase(1));
    assert(!films.update(1, Film(1, "Missing", 90)) && !films.isDirty());
    assert(films.insert(Film(10, "Film one", 110)));
    const Film* original = films.findById(10);
    assert(original != nullptr && original->title() == "Film one");
    assert(films.nextId() == 11 && films.isDirty());
    films.markSaved();
    assert(!films.insert(Film(10, "Duplicate", 90)) && !films.isDirty());
    assert(films.findById(10) == original && original->durationMinutes() == 110);
    expectThrow<std::invalid_argument>([&] { films.update(10, Film(11, "Wrong ID", 95)); });
    assert(!films.isDirty());
    assert(films.update(10, Film(10, "Renamed", 120)));
    assert(films.findById(10) == original && original->title() == "Renamed");
    assert(original->durationMinutes() == 120 && films.isDirty());
    for (EntityId id = 11; id <= 100; ++id) {
        assert(films.insert(Film(id, "Another film", 90)));
    }
    assert(films.findById(10) == original);
    assert(films.size() == 91 && films.nextId() == 101);
    films.markSaved();
    assert(!films.erase(999) && !films.isDirty());
    assert(films.erase(100) && films.nextId() == 101 && films.isDirty());
    assert(films.findById(100) == nullptr);
    std::size_t visited = 0;
    EntityId expectedId = 10;
    const FilmRepository& readOnly = films;
    for (const Film& film : readOnly) {
        assert(film.id() == expectedId++);
        assert(readOnly.findById(film.id()) == &film);
        ++visited;
    }
    assert(visited == films.size());
    for (EntityId id = 10; id < 100; ++id) {
        assert(films.erase(id));
    }
    assert(films.empty() && films.nextId() == 101);
    assert(films.insert(Film(films.nextId(), "Reused repository", 90)));
    assert(films.findById(101) != nullptr);
}

void testAllEntityRepositories() {
    AuditoriumRepository rooms;
    SeatRepository seats;
    ScreeningRepository screenings;
    TicketRepository tickets;
    assert(rooms.insert(Auditorium(1, "Room", 2, 3)));
    for (EntityId id = 1; id <= 6; ++id) {
        assert(seats.insert(Seat(id, 1, (id - 1) / 3 + 1, (id - 1) % 3 + 1)));
    }
    assert(screenings.insert(Screening(1, 1, 1, "2026-11-01 18:00", 80000)));
    assert(tickets.empty() && tickets.nextId() == 1 && !tickets.isDirty());
    assert(tickets.insert(Ticket(1, 1, 1, "2026-10-31 10:00", 80000)));
    assert(tickets.insert(Ticket(2, 1, 2, "2026-10-31 10:05", 80000)));
    assert(!tickets.insert(Ticket(2, 1, 3, "2026-10-31 10:06", 80000)));
    assert(tickets.size() == 2 && tickets.nextId() == 3 && tickets.isDirty());
    assert(rooms.findById(1)->capacity() == 6 && seats.size() == 6);
    assert(seats.findById(6)->seatNumber() == 3);
    assert(screenings.findById(1)->filmId() == 1);
    assert(tickets.findById(1)->seatId() == 1 && tickets.findById(999) == nullptr);
    std::int64_t revenue = 0;
    for (const Ticket& ticket : tickets) {
        revenue += ticket.unitPrice();
    }
    assert(revenue == 160000);
    tickets.markSaved();
    assert(!tickets.isDirty());
}

void testIdExhaustion() {
    FilmRepository films;
    const EntityId maximum = std::numeric_limits<EntityId>::max();
    assert(films.insert(Film(maximum, "Last ID", 90)));
    expectThrow<std::overflow_error>([&] { films.nextId(); });
    assert(films.erase(maximum));
    expectThrow<std::overflow_error>([&] { films.nextId(); });
}

void testInsertionRollback() {
    bool reachedSuccess = false;
    for (int budget = 0; budget < 40; ++budget) {
        FilmRepository films;
        for (EntityId id = 1; id <= 8; ++id) {
            films.insert(Film(id, "Existing", 90));
        }
        films.markSaved();
        const Film candidate(9, "New film with a name long enough to allocate a string", 90);
        const Film* original = films.findById(1);
        allocationsBeforeFailure = budget;
        bool failed = false;
        try {
            assert(films.insert(candidate));
        } catch (const std::bad_alloc&) {
            failed = true;
        }
        allocationsBeforeFailure = -1;
        if (failed) {
            assert(films.size() == 8 && films.nextId() == 9 && !films.isDirty());
            assert(films.findById(9) == nullptr);
        } else {
            assert(films.size() == 9 && films.nextId() == 10 && films.isDirty());
            reachedSuccess = true;
        }
        assert(films.findById(1) == original && original->title() == "Existing");
        std::size_t visited = 0;
        for (const Film& film : films) {
            assert(films.findById(film.id()) == &film);
            ++visited;
        }
        assert(visited == films.size());
        if (reachedSuccess) {
            break;
        }
    }
    assert(reachedSuccess);
}

void testUpdateRollback() {
    FilmRepository films;
    films.insert(Film(1, "Original", 90));
    films.markSaved();
    const Film replacement(1, "Replacement title long enough to allocate a string", 120);
    const Film* original = films.findById(1);
    allocationsBeforeFailure = 0;
    bool failed = false;
    try {
        films.update(1, replacement);
    } catch (const std::bad_alloc&) {
        failed = true;
    }
    allocationsBeforeFailure = -1;
    assert(failed && !films.isDirty());
    assert(films.findById(1) == original && original->title() == "Original");
    assert(original->durationMinutes() == 90 && films.size() == 1);
}

void testPreparedReplacement() {
    FilmRepository films;
    films.insert(Film(100, "Old", 90));
    films.erase(100);
    films.insert(Film(1, "Remaining", 90));
    films.markSaved();
    {
        FilmRepository staged;
        staged.insert(Film(2, "Replacement", 120));
        const Film* prepared = staged.findById(2);
        allocationsBeforeFailure = 0;
        films.replaceWith(staged);
        allocationsBeforeFailure = -1;
        assert(films.findById(2) == prepared && !films.findById(1));
        assert(staged.findById(1) && !staged.findById(2));
        assert(films.nextId() == 101 && films.isDirty());
        assert(!staged.isDirty());
    }
    assert(films.findById(2)->title() == "Replacement");
    films.markSaved();
    films.replaceWith(films);
    assert(films.size() == 1 && !films.isDirty());
    FilmRepository empty;
    films.replaceWith(empty);
    assert(films.empty() && films.nextId() == 101 && films.isDirty());
    assert(empty.findById(2) != nullptr);
}

template <typename T, typename = void>
struct HasErase : std::false_type {};

template <typename T>
struct HasErase<T, std::void_t<decltype(std::declval<T&>().erase(1))>> : std::true_type {};

template <typename T, typename = void>
struct HasTicketUpdate : std::false_type {};

template <typename T>
struct HasTicketUpdate<T, std::void_t<decltype(std::declval<T&>().update(1, std::declval<const Ticket&>()))>> : std::true_type {};

int main() {
    static_assert(std::is_same_v<decltype(*std::declval<FilmRepository&>().begin()), const Film&>);
    static_assert(std::is_same_v<decltype(*std::declval<const FilmRepository&>().begin()), const Film&>);
    static_assert(std::is_same_v<decltype(*std::declval<TicketRepository&>().begin()), const Ticket&>);
    static_assert(!std::is_copy_constructible_v<FilmRepository>);
    static_assert(!std::is_copy_assignable_v<FilmRepository>);
    static_assert(!std::is_move_constructible_v<FilmRepository>);
    static_assert(!std::is_move_assignable_v<FilmRepository>);
    static_assert(!std::is_copy_constructible_v<TicketRepository>);
    static_assert(!HasErase<TicketRepository>::value);
    static_assert(!HasTicketUpdate<TicketRepository>::value);
    static_assert(std::is_same_v<decltype(std::declval<FilmRepository&>().findById(1)), const Film*>);
    testCrudAndIds();
    testAllEntityRepositories();
    testIdExhaustion();
    testInsertionRollback();
    testUpdateRollback();
    testPreparedReplacement();
    std::cout << "Repository checks passed\n";
}
