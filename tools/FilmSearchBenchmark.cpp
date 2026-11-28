#include "database/CinemaDatabase.h"
#include "helper/DateTimeHelper.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
using Clock = std::chrono::steady_clock;
using Ids = std::vector<EntityId>;
volatile std::size_t consumed = 0;

Ids indexed(const FilmRepository& films, const std::string& query)
{
    LinkedList<const Film*> matches;
    films.searchTitle(query, matches);
    Ids result;
    for (const Film* film : matches) result.push_back(film->id());
    std::sort(result.begin(), result.end());
    return result;
}

Ids scan(const FilmRepository& films, const std::string& query)
{
    LinkedList<const Film*> matches;
    films.searchTitleSequential(query, matches);
    Ids result;
    for (const Film* film : matches) result.push_back(film->id());
    std::sort(result.begin(), result.end());
    return result;
}

template <typename Function>
double measure(Function function, int repetitions)
{
    for (int i = 0; i < 5; ++i) consumed += function().size();
    const auto start = Clock::now();
    for (int i = 0; i < repetitions; ++i) consumed += function().size();
    return std::chrono::duration<double, std::micro>(Clock::now() - start).count() / repetitions;
}

void require(bool valid, const char* message)
{
    if (!valid) throw std::runtime_error(message);
}

void validateSample(const CinemaDatabase& data)
{
    require(data.films.size() >= 1000 && data.rooms.size() == 20 && data.seats.size() == 2000
        && data.screenings.size() == 2000 && data.tickets.size() == 5000, "Wrong sample counts");
    std::vector<std::pair<EntityId, std::pair<std::int64_t, std::int64_t>>> intervals;
    for (const Screening& screening : data.screenings)
    {
        const auto start = date_time::toMinutes(screening.startTime());
        intervals.push_back({screening.auditoriumId(),
                            {start, start + screening.film().durationMinutes() + 15}});
    }
    std::sort(intervals.begin(), intervals.end());
    for (std::size_t i = 1; i < intervals.size(); ++i)
        if (intervals[i].first == intervals[i - 1].first)
            require(intervals[i - 1].second.second <= intervals[i].second.first, "Overlapping sample screenings");
    for (const auto& query : {"nguyen trai", "trai nguyen", "nguy", "guy", "nguyn trai"})
    {
        const auto ids = indexed(data.films, query);
        require(std::binary_search(ids.begin(), ids.end(), 1), "Missing required sample search result");
    }
}
}

int main(int argc, char** argv)
{
    try
    {
        if (argc < 2 || argc > 3) throw std::invalid_argument("Usage: FilmSearchBenchmark data-directory [repetitions]");
        const int repetitions = argc == 3 ? std::stoi(argv[2]) : 200;
        if (repetitions < 1 || repetitions > 100000) throw std::invalid_argument("Repetitions must be 1..100000");
        const auto start = Clock::now();
        CinemaDatabase data(argv[1]);
        require(data.load() == CinemaDatabase::LoadResult::Loaded, "Cannot load sample data");
        const double loadMs = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
        data.validate();
        validateSample(data);

        // Index build cost measured separately from full database load/validation.
        const auto indexStart = Clock::now();
        FilmRepository rebuilt;
        for (const Film& film : data.films) rebuilt.insert(film);
        const double buildMs = std::chrono::duration<double, std::milli>(Clock::now() - indexStart).count();
        const auto plainStart = Clock::now();
        Repository<Film> plain;
        for (const Film& film : data.films) plain.insert(film);
        const double plainMs = std::chrono::duration<double, std::milli>(Clock::now() - plainStart).count();
        std::cout << "# films=" << data.films.size() << ",rooms=" << data.rooms.size()
                  << ",seats=" << data.seats.size() << ",screenings=" << data.screenings.size()
                  << ",tickets=" << data.tickets.size() << ",repetitions=" << repetitions << '\n'
                  << "# full_load_ms=" << loadMs << ",film_load_with_title_index_ms=" << buildMs
                  << ",film_load_id_only_ms=" << plainMs << ",sizeof_film=" << sizeof(Film) << '\n';
        std::cout << "query,matches,index_us,seq_scan_us,speedup\n";
        std::cout << std::fixed << std::setprecision(3);
        for (const std::string query : {"nguyen trai", "trai nguyen", "nguy", "guy", "nguyn trai",
                                        "mat biec", u8"Chuyến Tàu Hoàng Hôn", "chuong", "ngu",
                                        "000777", "000777 nguyen", "zzzzzz"})
        {
            const Ids found = indexed(data.films, query);
            require(found == scan(data.films, query), "Index and same-rules scan disagree");
            const double indexUs = measure([&] { return indexed(data.films, query); }, repetitions);
            const double scanUs = measure([&] { return scan(data.films, query); }, repetitions);
            std::cout << query << ',' << found.size() << ',' << indexUs << ',' << scanUs << ','
                      << scanUs / indexUs << '\n';
        }
        std::cerr << "Validated dataset, required examples and index/scan equality; checksum=" << consumed << '\n';
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
