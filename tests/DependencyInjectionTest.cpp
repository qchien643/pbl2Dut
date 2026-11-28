#ifdef NDEBUG
#undef NDEBUG
#endif

#include "services/CatalogService.h"
#include "services/ScreeningService.h"
#include "services/TicketService.h"
#include "helper/TextNormalization.h"

#include <cassert>
#include <limits>
#include <list>
#include <stdexcept>

// Independent implementation: no Repository<T>, hash table or storage iterator.
template <typename T>
class TestRepository : public virtual IRepository<T>
{
public:
    EntityId nextId() const override
    {
        if (highestId_ == std::numeric_limits<EntityId>::max())
            throw std::overflow_error("IDs exhausted");
        return highestId_ + 1;
    }

    const T* findById(EntityId id) const override
    {
        for (const T& value : records_)
            if (value.id() == id) return &value;
        return nullptr;
    }

    bool insert(const T& value) override
    {
        if (findById(value.id())) return false;
        records_.push_back(value);
        if (value.id() > highestId_) highestId_ = value.id();
        return true;
    }

    bool update(EntityId id, const T& value) override
    {
        if (id != value.id()) throw std::invalid_argument("ID mismatch");
        for (T& current : records_)
        {
            if (current.id() == id)
            {
                T replacement(value);
                using std::swap;
                swap(current, replacement);
                return true;
            }
        }
        return false;
    }

    bool erase(EntityId id) override
    {
        for (auto it = records_.begin(); it != records_.end(); ++it)
        {
            if (it->id() == id)
            {
                records_.erase(it);
                return true;
            }
        }
        return false;
    }

    void list(LinkedList<const T*>& result) const override
    {
        LinkedList<const T*> snapshot;
        for (const T& value : records_) snapshot.pushBack(&value);
        result.swap(snapshot);
    }

private:
    std::list<T> records_;
    EntityId highestId_ = 0;
};

class TestFilmRepository : public TestRepository<Film>, public IFilmRepository
{
public:
    void searchTitle(const std::string& text, LinkedList<const Film*>& result) const override
    {
        // ponytail: DI-only substring fake; full search behavior is checked in FilmSearchTest.
        const auto query = text_normalization::normalize(text);
        LinkedList<const Film*> records;
        list(records);
        LinkedList<const Film*> matches;
        for (const Film* film : records)
            if (film->normalizedTitle().find(query) != std::u32string::npos) matches.pushBack(film);
        result.swap(matches);
    }
};

// This test seeds rooms directly; fail explicitly if a room write is requested.
class UnusedRoomStorage : public IRoomStorage
{
public:
    void add(const Auditorium&, const LinkedList<Seat>&) override
    { throw std::logic_error("Unexpected room write"); }
    void update(const Auditorium&, const LinkedList<Seat>&) override
    { throw std::logic_error("Unexpected room write"); }
};

std::string testClock() { return "2026-11-18 09:00"; }

int main()
{
    TestFilmRepository films;
    TestRepository<Auditorium> rooms;
    TestRepository<Seat> seats;
    TestRepository<Screening> screenings;
    TestRepository<Ticket> tickets;
    UnusedRoomStorage roomStorage;
    CatalogService catalog(films, rooms, seats, screenings, roomStorage);
    ScreeningService schedule(films, rooms, screenings, tickets, testClock);
    TicketService sales(seats, screenings, tickets, testClock);

    const EntityId filmId = catalog.addFilm("DIP", 90);
    LinkedList<Film> found;
    catalog.films("dip", found);
    assert(found.size() == 1 && (*found.begin()).id() == filmId);
    assert(rooms.insert(Auditorium(1, "Room", 1, 1)));
    assert(seats.insert(Seat(1, *rooms.findById(1), 1, 1)));
    const EntityId screeningId = schedule.add(filmId, 1, "2026-11-18 10:00", 50000);
    const EntityId ticketId = sales.sell(screeningId, 1, true);
    assert(&screenings.findById(screeningId)->film() == films.findById(filmId));
    assert(&tickets.findById(ticketId)->seat() == seats.findById(1));

    LinkedList<const Film*> snapshot;
    films.list(snapshot);
    const Film* original = films.findById(filmId);
    assert(*snapshot.begin() == original);
    catalog.updateFilm(filmId, "Renamed", 90);
    assert(films.findById(filmId) == original && original->title() == "Renamed");
    catalog.films("dip", found);
    assert(found.empty());
    catalog.films("renamed", found);
    assert(found.size() == 1 && (*found.begin()).id() == filmId);
    catalog.films("", found);
    assert(found.size() == 1 && (*found.begin()).id() == filmId);
    assert(sales.find(ticketId).ticket.unitPrice() == 50000);
    assert(sales.revenue("2026-11-18").ticketCount == 1);
    assert(sales.seatMap(screeningId)->at(0, 0).sold);

    bool rejected = false;
    try { sales.sell(screeningId, 1, true); }
    catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
    rejected = false;
    try { catalog.eraseFilm(filmId); }
    catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
}
