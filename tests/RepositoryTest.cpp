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
    FilmRepository films;
    AuditoriumRepository rooms;
    SeatRepository seats;
    ScreeningRepository screenings;
    TicketRepository tickets;
    assert(films.insert(Film(1, "Film", 120)));
    assert(rooms.insert(Auditorium(1, "Room", 2, 3)));
    const Film& film = *films.findById(1);
    const Auditorium& room = *rooms.findById(1);
    for (EntityId id = 1; id <= 6; ++id) {
        assert(seats.insert(Seat(id, room, (id - 1) / 3 + 1, (id - 1) % 3 + 1)));
    }
    assert(screenings.insert(Screening(1, film, room, "2026-11-01 18:00", 80000)));
    const Screening& screening = *screenings.findById(1);
    assert(tickets.empty() && tickets.nextId() == 1 && !tickets.isDirty());
    assert(tickets.insert(Ticket(1, screening, *seats.findById(1), "2026-10-31 10:00", 80000)));
    assert(tickets.insert(Ticket(2, screening, *seats.findById(2), "2026-10-31 10:05", 80000)));
    assert(!tickets.insert(Ticket(2, screening, *seats.findById(3), "2026-10-31 10:06", 80000)));
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

    // Storage supports the same CRUD contract for tickets; business limits live in services.
    IRepository<Ticket>& ticketStore = tickets;
    const Ticket* original = ticketStore.findById(1);
    assert(ticketStore.update(1, Ticket(1, screening, *seats.findById(1),
                                        "2026-10-31 10:00", 90000)));
    assert(ticketStore.findById(1) == original && original->unitPrice() == 90000);
    assert(tickets.isDirty());
    tickets.markSaved();
    assert(ticketStore.erase(2) && ticketStore.findById(2) == nullptr);
    assert(tickets.size() == 1 && tickets.nextId() == 3 && tickets.isDirty());
    LinkedList<const Ticket*> snapshot;
    ticketStore.list(snapshot);
    assert(snapshot.size() == 1 && *snapshot.begin() == original);
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
    for (int budget = 0; budget < 600; ++budget) {
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
        LinkedList<const Film*> matches;
        films.searchTitle("allocate", matches);
        assert(matches.size() == (failed ? 0 : 1));
        films.searchTitle("existing", matches);
        assert(matches.size() == 8);
        if (reachedSuccess) {
            break;
        }
    }
    assert(reachedSuccess);
}

void testUpdateRollback() {
    bool reachedSuccess = false;
    for (int budget = 0; budget < 600; ++budget) {
        FilmRepository films;
        films.insert(Film(1, "Original", 90));
        films.markSaved();
        const Film replacement(1, "Replacement title long enough to allocate a string", 120);
        const Film* original = films.findById(1);
        allocationsBeforeFailure = budget;
        bool failed = false;
        try { films.update(1, replacement); }
        catch (const std::bad_alloc&) { failed = true; }
        allocationsBeforeFailure = -1;
        assert(films.findById(1) == original && films.size() == 1);
        assert(original->title() == (failed ? "Original" : replacement.title()));
        assert(original->normalizedTitle() == (failed ? U"original" : replacement.normalizedTitle()));
        assert(original->durationMinutes() == (failed ? 90 : 120));
        assert(films.isDirty() == !failed);
        LinkedList<const Film*> matches;
        films.searchTitle("replacement", matches);
        assert(matches.size() == (failed ? 0 : 1));
        films.searchTitle("original", matches);
        assert(matches.size() == (failed ? 1 : 0));
        if (!failed) { reachedSuccess = true; break; }
    }
    assert(reachedSuccess);

    FilmRepository films;
    films.insert(Film(1, "Erase without allocation", 90));
    allocationsBeforeFailure = 0;
    assert(films.erase(1));
    allocationsBeforeFailure = -1;
    LinkedList<const Film*> matches;
    films.searchTitle("allocation", matches);
    assert(matches.empty());
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

void testCombinedFilmContract() {
    FilmRepository repository;
    IFilmRepository& films = repository;
    Repository<Film>& storage = repository;
    IRepository<Film>& crud = films;
    assert(&crud == static_cast<IRepository<Film>*>(&storage));

    assert(films.insert(Film(1, u8"Nguyễn Trãi", 90)));
    assert(!crud.insert(Film(1, "Duplicate ID", 90)));
    assert(films.nextId() == 2 && crud.findById(1) == repository.findById(1));
    LinkedList<const Film*> result;
    const IFilmRepository& query = films;
    query.searchTitle("nguyen trai", result);
    assert(result.size() == 1 && *result.begin() == crud.findById(1));

    // Writes through the generic contract still dispatch to FilmRepository's index updates.
    const Film* original = crud.findById(1);
    assert(crud.update(1, Film(1, u8"Mắt Biếc", 100)));
    assert(crud.findById(1) == original);
    query.searchTitle("nguyen trai", result);
    assert(result.empty());
    query.searchTitle("mat biec", result);
    assert(result.size() == 1 && *result.begin() == original);
    films.list(result);
    assert(result.size() == 1 && *result.begin() == original);
    assert(crud.erase(1));
    query.searchTitle("mat biec", result);
    assert(result.empty() && crud.findById(1) == nullptr);
}

int main() {
    static_assert(std::is_same_v<decltype(*std::declval<FilmRepository&>().begin()), const Film&>);
    static_assert(std::is_same_v<decltype(*std::declval<const FilmRepository&>().begin()), const Film&>);
    static_assert(std::is_same_v<decltype(*std::declval<TicketRepository&>().begin()), const Ticket&>);
    static_assert(!std::is_copy_constructible_v<FilmRepository>);
    static_assert(!std::is_copy_assignable_v<FilmRepository>);
    static_assert(!std::is_move_constructible_v<FilmRepository>);
    static_assert(!std::is_move_assignable_v<FilmRepository>);
    static_assert(!std::is_copy_constructible_v<TicketRepository>);
    static_assert(std::is_base_of_v<Repository<Ticket>, TicketRepository>);
    static_assert(std::is_base_of_v<IRepository<Ticket>, TicketRepository>);
    static_assert(std::is_base_of_v<IRepository<Film>, IFilmRepository>);
    static_assert(std::is_convertible_v<FilmRepository*, IFilmRepository*>);
    static_assert(std::is_convertible_v<FilmRepository*, IRepository<Film>*>);
    static_assert(std::is_same_v<decltype(std::declval<FilmRepository&>().findById(1)), const Film*>);
    testCrudAndIds();
    testAllEntityRepositories();
    testIdExhaustion();
    testInsertionRollback();
    testUpdateRollback();
    testPreparedReplacement();
    testCombinedFilmContract();
    std::cout << "Repository checks passed\n";
}
