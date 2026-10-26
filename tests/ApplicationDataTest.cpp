#ifdef NDEBUG
#undef NDEBUG
#endif

#include "ApplicationData.h"
#include "persistence/TxtPersistence.h"
#include "services/CatalogService.h"
#include "services/ScreeningService.h"
#include "services/TicketService.h"

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <new>
#include <type_traits>

namespace fs = std::filesystem;

int allocationsBeforeFailure = -1;

void* operator new(std::size_t size) {
    if (allocationsBeforeFailure == 0) {
        allocationsBeforeFailure = -1; // One-shot failure lets error reporting/rollback run.
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

struct ApplicationDataTestAccess {
    static void hook(ApplicationData& data, void (*callback)(const char*, std::size_t)) {
        data.beforeFileStep_ = callback;
    }
};

struct TemporaryDirectory {
    fs::path path;
    TemporaryDirectory() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        for (unsigned attempt = 0; attempt < 1000; ++attempt) {
            path = fs::temp_directory_path() / ("pbl2-data-test-" + std::to_string(stamp)
                                               + "-" + std::to_string(attempt));
            if (fs::create_directory(path)) {
                return;
            }
        }
        throw std::runtime_error("Cannot create unique test directory");
    }
    ~TemporaryDirectory() {
        std::error_code error;
        fs::remove_all(path, error); // Only this test's exclusively created directory.
    }
};

std::string read(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    assert(input);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

void write(const fs::path& path, const std::string& value) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << value;
    output.close();
    assert(output);
}

template <typename Function>
void expectError(Function action, const std::string& message) {
    bool threw = false;
    try {
        action();
    } catch (const std::exception& error) {
        threw = true;
        if (std::string(error.what()).find(message) == std::string::npos) {
            std::cerr << "Expected: " << message << "\nActual: " << error.what() << '\n';
            assert(false);
        }
    }
    assert(threw);
}

std::string fixedClock() { return "2020-01-01 10:00"; }

void populate(ApplicationData& data) {
    assert(data.load(true) == ApplicationData::LoadResult::Loaded);
    CatalogService catalog(data.films, data.rooms, data.seats, data.screenings);
    ScreeningService schedule(data.films, data.rooms, data.screenings, data.tickets, fixedClock);
    TicketService sales(data.films, data.rooms, data.seats, data.screenings, data.tickets, fixedClock);
    assert(catalog.addFilm(u8"Phim Vi\u1ec7t", 120) == 1);
    assert(catalog.addRoom(u8"Ph\u00f2ng 1", 2, 2) == 1);
    assert(catalog.addRoom("Room 2", 1, 1) == 2);
    assert(schedule.add(1, 1, "2020-01-02 23:00", 80000) == 1);
    assert(schedule.add(1, 1, "2020-01-03 01:15", 90000) == 2);
    assert(sales.sell(1, 1, true) == 1);
}

void assertOriginal(const ApplicationData& data) {
    assert(data.isReady() && !data.isDirty());
    assert(data.films.size() == 1 && data.films.findById(1)->title() == u8"Phim Vi\u1ec7t");
    assert(data.rooms.size() == 2 && data.seats.size() == 5);
    assert(data.screenings.size() == 2 && data.tickets.size() == 1);
    assert(data.seats.findById(4)->rowNumber() == 2);
    assert(data.tickets.findById(1)->unitPrice() == 80000);
    assert(data.tickets.nextId() == 2);
}

void testEmptyAndRoundTrip() { // FR-DATA-07/10, SAT-08/16.
    TemporaryDirectory directory;
    ApplicationData data(directory.path);
    expectError([&] { data.save(); }, "before saving");
    assert(data.load() == ApplicationData::LoadResult::NeedsEmptyConfirmation);
    assert(!data.isReady() && fs::is_empty(directory.path));
    populate(data);
    assert(data.isDirty() && fs::is_empty(directory.path));
    data.save();
    assertOriginal(data);
    ApplicationData reloaded(directory.path);
    reloaded.load(); // Historical screenings are valid; no comparison against today's clock.
    assertOriginal(reloaded);
    expectError([&] { reloaded.load(); }, "fresh");
    CatalogService catalog(reloaded.films, reloaded.rooms, reloaded.seats, reloaded.screenings);
    catalog.updateFilm(1, "New title", 120);
    catalog.eraseRoom(2);
    catalog.addFilm("New film", 90);
    const auto oldText = read(directory.path / "films.txt");
    assert(oldText.find("New title") == std::string::npos);
    reloaded.save();
    assert(read(directory.path / ".cms-backup-2" / "old" / "films.txt") == oldText);
    ApplicationData next(directory.path);
    next.load();
    assert(next.films.size() == 2 && next.films.findById(1)->title() == "New title");
    assert(next.rooms.size() == 1 && next.seats.size() == 4 && !next.seats.findById(5));
    assert(next.films.nextId() == 3 && next.seats.nextId() == 5);
    const auto unchanged = read(directory.path / "tickets.txt");
    next.save(); // Full rewrite even when isDirty() is false.
    assert(read(directory.path / "tickets.txt") == unchanged);
    assert(fs::exists(directory.path / ".cms-backup-3"));

    TemporaryDirectory emptyDirectory;
    ApplicationData empty(emptyDirectory.path);
    empty.load(true);
    empty.save();
    ApplicationData emptyAgain(emptyDirectory.path);
    assert(emptyAgain.load() == ApplicationData::LoadResult::Loaded);
    assert(emptyAgain.isReady() && emptyAgain.films.empty() && !emptyAgain.isDirty());
}

void testBadInput() { // FR-DATA-04, SAT-13/14.
    struct BadInput {
        const char* file;
        const char* body;
        const char* message;
    };
    const BadInput cases[] = {
        {"films.txt", "", "films.txt:1"},
        {"films.txt", "wrong\n", "Incorrect header"},
        {"films.txt", "film_id|title|duration_minutes\n1|Film|120\n1|Other|90\n", "films.txt:3"},
        {"films.txt", "film_id|title|duration_minutes\n1|Film|120junk\n", "films.txt:2"},
        {"films.txt", "film_id|title|duration_minutes\n1|Film|9223372036854775808\n", "int64"},
        {"films.txt", "film_id|title|duration_minutes\n1|Film\n", "field count"},
        {"films.txt", "film_id|title|duration_minutes\n1|Film|120|extra\n", "field count"},
        {"films.txt", "film_id|title|duration_minutes\n\n", "films.txt:2"},
        {"films.txt", "film_id|title|duration_minutes\n1|Bad\xc0\xaf|120\n", "UTF-8"},
        {"films.txt", "film_id|title|duration_minutes\n1|Bad\xed\xa0\x80|120\n", "UTF-8"},
        {"films.txt", "film_id|title|duration_minutes\n1|Bad\xf4\x90\x80\x80|120\n", "UTF-8"},
        {"auditoriums.txt", "auditorium_id|name|row_count|seats_per_row\n1|Room|0|2\n", "positive"},
        {"seats.txt", "seat_id|auditorium_id|row_number|seat_number\n1|99|1|1\n", "seats.txt:2"},
        {"seats.txt", "seat_id|auditorium_id|row_number|seat_number\n1|1|3|1\n", "out-of-range"},
        {"seats.txt", "seat_id|auditorium_id|row_number|seat_number\n1|1|1|1\n2|1|1|1\n", "Duplicate room"},
        {"seats.txt", "seat_id|auditorium_id|row_number|seat_number\n1|1|1|1\n", "complete rectangular"},
        {"seats.txt", "seat_id|auditorium_id|row_number|seat_number\n1|1|1|1\n", "auditoriums.txt:2"},
        {"seats.txt", "seat_id|auditorium_id|row_number|seat_number\n1|1|1|1\n2|99|1|2\n", "seats.txt:3"},
        {"screenings.txt", "screening_id|film_id|auditorium_id|start_time|ticket_price\n1|99|1|2020-01-02 23:00|80000\n", "screenings.txt:2"},
        {"screenings.txt", "screening_id|film_id|auditorium_id|start_time|ticket_price\n1|1|99|2020-01-02 23:00|80000\n", "screenings.txt:2"},
        {"screenings.txt", "screening_id|film_id|auditorium_id|start_time|ticket_price\n1|1|1|2020-02-30 23:00|80000\n", "Day"},
        {"screenings.txt", "screening_id|film_id|auditorium_id|start_time|ticket_price\n1|1|1|2020-01-02 23:00|80000\n2|1|1|2020-01-03 01:14|80000\n", "overlaps"},
        {"screenings.txt", "screening_id|film_id|auditorium_id|start_time|ticket_price\n1|1|1|2020-01-02 23:00|80000\n2|1|99|2020-01-03 01:15|80000\n", "screenings.txt:3"},
        {"tickets.txt", "ticket_id|screening_id|seat_id|sold_at|unit_price\n1|1|1|2020-01-02 23:00|80000\n", "before screening"},
        {"tickets.txt", "ticket_id|screening_id|seat_id|sold_at|unit_price\n1|1|1|2020-01-03 00:00|80000\n", "before screening"},
        {"tickets.txt", "ticket_id|screening_id|seat_id|sold_at|unit_price\n1|99|1|2020-01-01 10:00|80000\n", "missing"},
        {"tickets.txt", "ticket_id|screening_id|seat_id|sold_at|unit_price\n1|1|99|2020-01-01 10:00|80000\n", "missing"},
        {"tickets.txt", "ticket_id|screening_id|seat_id|sold_at|unit_price\n1|1|5|2020-01-01 10:00|80000\n", "wrong room"},
        {"tickets.txt", "ticket_id|screening_id|seat_id|sold_at|unit_price\n1|1|1|2020-01-01 10:00|80000\n2|1|1|2020-01-01 11:00|80000\n", "Duplicate screening-seat"},
        {"tickets.txt", "ticket_id|screening_id|seat_id|sold_at|unit_price\n1|1|1|2020-01-01 10:00|80000\n2|1|1|2020-01-01 11:00|80000\n", "tickets.txt:3"},
    };
    for (const auto& test : cases) {
        TemporaryDirectory directory;
        ApplicationData original(directory.path);
        populate(original);
        original.save();
        write(directory.path / test.file, test.body);
        ApplicationData data(directory.path);
        expectError([&] { data.load(true); }, test.message);
        assert(!data.isReady() && !data.isDirty());
        assert(data.films.empty() && data.rooms.empty() && data.seats.empty());
        assert(data.screenings.empty() && data.tickets.empty());
        assert(data.films.nextId() == 1 && data.tickets.nextId() == 1);
        expectError([&] { data.save(); }, "before saving");
        assert(read(directory.path / test.file) == test.body);
    }
}

void testFileSetAndEncoding() {
    TemporaryDirectory directory;
    ApplicationData original(directory.path);
    populate(original);
    original.save();
    const auto films = read(directory.path / "films.txt");
    std::string windows = "\xef\xbb\xbf";
    for (char character : films) {
        windows += character == '\n' ? "\r\n" : std::string(1, character);
    }
    windows.erase(windows.size() - 2); // A final newline is optional.
    write(directory.path / "films.txt", windows);
    ApplicationData crlf(directory.path);
    crlf.load();
    assertOriginal(crlf);
    crlf.save();
    assert(read(directory.path / "films.txt") == films);
    fs::rename(directory.path / "films.txt", directory.path / "hidden.txt");
    ApplicationData missing(directory.path);
    expectError([&] { missing.load(true); }, "Incomplete");
    fs::create_directory(directory.path / "films.txt");
    expectError([&] { missing.load(); }, "regular file");
    fs::remove(directory.path / "films.txt");
    fs::create_symlink(directory.path / "hidden.txt", directory.path / "films.txt");
    expectError([&] { missing.load(); }, "symlink");
    fs::remove(directory.path / "films.txt");
    fs::rename(directory.path / "hidden.txt", directory.path / "films.txt");
    fs::create_directory(directory.path / ".cms-save");
    expectError([&] { missing.load(); }, "Unfinished");

    TemporaryDirectory empty;
    fs::create_directory(empty.path / ".cms-backup-1");
    ApplicationData history(empty.path);
    expectError([&] { history.load(true); }, "backups exist");
    ApplicationData absent(empty.path / "absent");
    expectError([&] { absent.load(true); }, "must exist");
}

const char* failurePhase = "";
std::size_t failureIndex = 0;
bool failRestore = false;
bool failAllocation = false;
void failFileStep(const char* phase, std::size_t index) {
    if ((std::string(phase) == failurePhase && index == failureIndex)
        || (failRestore && std::string(phase) == "restore" && index == 0)) {
        if (failAllocation) {
            throw std::bad_alloc();
        }
        throw std::runtime_error("Injected filesystem failure");
    }
}

void testSaveFailures() { // FR-DATA-08/09, SAT-15 (storage half).
    const char* names[] = {"films.txt", "auditoriums.txt", "seats.txt", "screenings.txt", "tickets.txt"};
    for (bool firstSave : {false, true}) {
        for (const char* phase : {"write", "backup", "install", "archive"}) {
            if (firstSave && std::string(phase) == "backup") {
                continue;
            }
            for (std::size_t index = 0; index < (std::string(phase) == "archive" ? 1u : 5u); ++index) {
                TemporaryDirectory directory;
                ApplicationData data(directory.path);
                populate(data);
                std::string before[5];
                if (!firstSave) {
                    data.save();
                    for (std::size_t i = 0; i < 5; ++i) {
                        before[i] = read(directory.path / names[i]);
                    }
                }
                CatalogService catalog(data.films, data.rooms, data.seats, data.screenings);
                catalog.updateFilm(1, "Changed", 120);
                failurePhase = phase;
                failureIndex = index;
                ApplicationDataTestAccess::hook(data, failFileStep);
                expectError([&] { data.save(); }, "disk state restored");
                assert(data.isReady() && data.isDirty() && !data.needsManualRecovery());
                assert(data.films.findById(1)->title() == "Changed" && data.tickets.size() == 1);
                assert(fs::exists(directory.path / ".cms-save"));
                for (std::size_t i = 0; i < 5; ++i) {
                    if (firstSave) {
                        assert(!fs::exists(directory.path / names[i]));
                    } else {
                        assert(read(directory.path / names[i]) == before[i]);
                        if (std::string(phase) == "install" || std::string(phase) == "archive") {
                            assert(read(directory.path / ".cms-save" / "old" / names[i]) == before[i]);
                        }
                    }
                }
                ApplicationData reopening(directory.path);
                expectError([&] { reopening.load(true); }, "Unfinished");
                ApplicationDataTestAccess::hook(data, nullptr);
                data.save();
                assert(!data.isDirty() && !fs::exists(directory.path / ".cms-save"));
                ApplicationData saved(directory.path);
                saved.load();
                assert(saved.films.findById(1)->title() == "Changed");
            }
        }
    }
}

void testFailedRecoveryAndValidation() {
    TemporaryDirectory directory;
    ApplicationData data(directory.path);
    populate(data);
    data.save();
    const auto original = read(directory.path / "films.txt");
    CatalogService catalog(data.films, data.rooms, data.seats, data.screenings);
    catalog.updateFilm(1, "Changed", 120);
    failurePhase = "install";
    failureIndex = 2;
    failRestore = true;
    ApplicationDataTestAccess::hook(data, failFileStep);
    expectError([&] { data.save(); }, "ROLLBACK FAILED");
    failRestore = false;
    assert(data.isDirty() && data.needsManualRecovery());
    assert(read(directory.path / ".cms-save" / "old" / "films.txt") == original);
    ApplicationDataTestAccess::hook(data, nullptr);
    expectError([&] { data.save(); }, "manual recovery");
    ApplicationData reopening(directory.path);
    expectError([&] { reopening.load(); }, "Unfinished");
    data.retryRecovery();
    assert(!data.needsManualRecovery() && data.isDirty());
    assert(read(directory.path / "films.txt") == original);
    data.save();
    assert(!data.isDirty());
    reopening.load();
    assert(reopening.films.findById(1)->title() == "Changed");

    TemporaryDirectory invalidDirectory;
    ApplicationData invalid(invalidDirectory.path);
    populate(invalid);
    invalid.save();
    const auto tickets = read(invalidDirectory.path / "tickets.txt");
    invalid.tickets.insert(Ticket(2, 1, 2, "2020-01-02 23:00", 80000));
    expectError([&] { invalid.save(); }, "before screening");
    assert(invalid.isDirty() && !fs::exists(invalidDirectory.path / ".cms-save"));
    assert(read(invalidDirectory.path / "tickets.txt") == tickets);
}

fs::path injectedDirectory;
void breakRealWrite(const char* phase, std::size_t index) {
    if (std::string(phase) == "write" && index == 2) {
        fs::create_directory(injectedDirectory / ".cms-save" / "new" / "seats.txt");
    }
}
void breakRealInstall(const char* phase, std::size_t index) {
    if (std::string(phase) == "install" && index == 2) {
        fs::remove(injectedDirectory / ".cms-save" / "new" / "seats.txt");
    }
}

void testRealIoFailures() {
    for (auto hook : {breakRealWrite, breakRealInstall}) {
        TemporaryDirectory directory;
        injectedDirectory = directory.path;
        ApplicationData data(directory.path);
        populate(data);
        data.save();
        const auto previous = read(directory.path / "films.txt");
        data.films.update(1, Film(1, "Changed", 120));
        ApplicationDataTestAccess::hook(data, hook);
        expectError([&] { data.save(); }, "disk state restored");
        assert(data.isDirty() && read(directory.path / "films.txt") == previous);
    }
}

void testAllocationFailures() {
    TemporaryDirectory directory;
    ApplicationData original(directory.path);
    populate(original);
    original.save();
    // Exercise our codec; global allocation failure inside libstdc++'s directory
    // iterator can terminate in a noexcept internal function before reaching us.
    bool loaded = false;
    for (int budget = 0; budget < 3000; ++budget) {
        FilmRepository staged;
        allocationsBeforeFailure = budget;
        bool failed = false;
        try {
            txt_persistence::load(directory.path / "films.txt", staged);
        } catch (const std::exception&) {
            failed = true;
        }
        allocationsBeforeFailure = -1;
        if (failed) {
            assertOriginal(original);
        } else {
            assert(staged.size() == 1 && staged.findById(1)->title() == u8"Phim Vi\u1ec7t");
            loaded = true;
            break;
        }
    }
    assert(loaded);

    bool saved = false;
    for (int budget = 0; budget < 4000; ++budget) {
        allocationsBeforeFailure = budget;
        bool failed = false;
        try {
            txt_persistence::save(directory.path / "temporary.txt", original.films);
        } catch (const std::exception&) {
            failed = true;
        }
        allocationsBeforeFailure = -1;
        if (failed) {
            assertOriginal(original);
        } else {
            assert(read(directory.path / "temporary.txt") == read(directory.path / "films.txt"));
            saved = true;
            break;
        }
    }
    assert(saved);
    failAllocation = true;
    testSaveFailures();
    failAllocation = false;
}

void testDemoData() {
    const auto demo = std::filesystem::path(__FILE__).parent_path().parent_path() / "data" / "demo";
    ApplicationData data(demo);
    assert(data.load() == ApplicationData::LoadResult::Loaded);
    assert(data.films.size() == 2 && data.rooms.size() == 2 && data.seats.size() == 18);
    assert(data.screenings.size() == 3 && data.tickets.size() == 2 && !data.isDirty());
}

int main() {
    static_assert(!std::is_copy_constructible_v<ApplicationData>);
    static_assert(!std::is_move_constructible_v<ApplicationData>);
    testEmptyAndRoundTrip();
    testBadInput();
    testFileSetAndEncoding();
    testSaveFailures();
    testFailedRecoveryAndValidation();
    testRealIoFailures();
    testAllocationFailures();
    testDemoData();
    std::cout << "ApplicationData checks passed\n";
}
