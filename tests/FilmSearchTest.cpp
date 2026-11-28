#ifdef NDEBUG
#undef NDEBUG
#endif

#include "repositories/FilmRepository.h"
#include "helper/TextNormalization.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>

std::vector<EntityId> search(const FilmRepository& repo, const std::string& query)
{
    LinkedList<const Film*> result;
    repo.searchTitle(query, result);
    std::vector<EntityId> ids;
    for (const Film* film : result) ids.push_back(film->id());
    return ids;
}

void testRequirementsAndUnicode()
{
    FilmRepository films;
    films.insert(Film(1, u8"Nguyễn Trãi", 100));
    films.insert(Film(2, u8"Người Nhện", 100));
    films.insert(Film(3, "nguxyz abc def", 100));
    for (const std::string query : {"nguyen trai", "trai nguyen", "nguy", "guy", "nguyn trai",
                                   u8"NGUYỄN TRÃI", u8"Nguye\u0302\u0303n Tra\u0303i"})
        assert(search(films, query) == std::vector<EntityId>{1});
    assert(text_normalization::normalize(u8"ĐẶNG-THỊ  ÁNH") == U"dang thi anh");
    assert(text_normalization::normalize(u8"Nguyễn") == text_normalization::normalize(u8"Nguye\u0302\u0303n"));
    assert(text_normalization::normalize(" -- ABC \t ") == U"abc");
    assert(text_normalization::normalize("---").empty());
    assert(search(films, "nhne").empty()); // No common trigram: deliberately outside scope.
    assert(search(films, "---").empty());
    assert(search(films, " ").size() == 3);
    assert(search(films, "nguyn trxx").empty());
    FilmRepository spelling;
    spelling.insert(Film(1, "nguyen", 90));
    spelling.insert(Film(2, "abcdef", 90));
    for (const std::string query : {"nguyn", "nguyenn", "nguyem"})
        assert(search(spelling, query) == std::vector<EntityId>{1});
    assert(search(spelling, "abcedf") == std::vector<EntityId>{2});
    assert(search(spelling, "ngxyn").empty());

    films.insert(Film(4, u8"東京映画", 90));
    assert(search(films, u8"東京映") == std::vector<EntityId>{4});
    for (const std::string invalid : {std::string("nh"), std::string("\xc0\xaf", 2),
                                     std::string("\xed\xa0\x80", 3), std::string("\xf4\x90\x80\x80", 4)})
    {
        LinkedList<const Film*> output;
        output.pushBack(films.findById(1));
        bool failed = false;
        try { films.searchTitle(invalid, output); }
        catch (const std::invalid_argument&) { failed = true; }
        assert(failed && output.size() == 1 && *output.begin() == films.findById(1));
        failed = false;
        try { films.searchTitleSequential(invalid, output); }
        catch (const std::invalid_argument&) { failed = true; }
        assert(failed && output.size() == 1 && *output.begin() == films.findById(1));
    }
    for (const std::string query : {"", " ", "---", "trai nguyen", "nguyn trai"})
    {
        LinkedList<const Film*> sequential;
        films.searchTitleSequential(query, sequential);
        std::vector<EntityId> ids;
        for (const Film* film : sequential) ids.push_back(film->id());
        assert(search(films, query) == ids);
    }
}

void testCrud()
{
    FilmRepository films;
    films.insert(Film(10, u8"Mắt Biếc", 90));
    films.insert(Film(20, u8"Mắt Biếc Khác", 90));
    const Film* address = films.findById(10);
    assert(address->normalizedTitle() == U"mat biec");
    assert(films.update(10, Film(10, u8"Mắt Bão", 90)));
    assert(films.findById(10) == address);
    assert(address->normalizedTitle() == U"mat bao");
    assert(search(films, "biec") == std::vector<EntityId>{20});
    assert(search(films, "bao") == std::vector<EntityId>{10});
    assert(films.update(10, Film(10, u8"MẮT BÃO", 120)));
    assert(address->normalizedTitle() == U"mat bao");
    assert(search(films, "bao") == std::vector<EntityId>{10});
    assert(films.erase(20) && search(films, "biec").empty());
    assert(search(films, "mat") == std::vector<EntityId>{10});
    FilmRepository staged;
    staged.insert(Film(1, u8"Nguyễn Trãi", 100));
    const Film* stagedAddress = staged.findById(1);
    films.replaceWith(staged);
    assert(search(films, "nguyen trai") == std::vector<EntityId>{1});
    assert(films.findById(1) == stagedAddress && search(films, "bao").empty());
    assert(search(staged, "bao") == std::vector<EntityId>{10});
    films.replaceWith(films);
    assert(search(films, "nguyen trai") == std::vector<EntityId>{1});
    FilmRepository empty;
    films.replaceWith(empty);
    assert(search(films, "nguyen").empty());

    // Posting pointers refer to stored nodes, not the temporary input Film.
    films.insert(Film(40, "Shared title", 90));
    films.insert(Film(2, "Shared title", 90));
    films.insert(Film(30, "Shared title", 90));
    assert(search(films, "shared") == (std::vector<EntityId>{2, 30, 40}));
    LinkedList<const Film*> borrowed;
    films.searchTitle("shared", borrowed);
    for (const Film* film : borrowed) assert(film == films.findById(film->id()));
    assert(films.erase(30));
    assert(search(films, "shared") == (std::vector<EntityId>{2, 40}));
    assert(films.update(40, Film(40, "Sharedd title", 90)));
    assert(search(films, "sharedd") == (std::vector<EntityId>{2, 40}));
}

void testDifferential()
{
    FilmRepository films;
    std::mt19937 random(643);
    const std::string titles[] = {u8"Nguyễn Trãi", u8"Người Nhện", u8"Mắt Biếc", "aaa aaaa",
                                  "abc def ghi", "abcdefghi", "xyz abcy abcdef", "Nguyen Nguyen"};
    for (int id = 1; id <= 120; ++id)
        films.insert(Film(id, titles[random() % 8] + " " + std::to_string(1000 + id), 90));
    const std::string queries[] = {"nguy", "nguyn trai", "trai nguyen", "nhen", "abcdef",
                                   "aaaa", "aaa ghi", "abc", "abcedf", "xyz", "1001", "notfound"};
    for (int step = 0; step < 80; ++step)
    {
        const int id = 1 + random() % 120;
        if (step % 3 == 0) films.erase(id);
        else if (films.findById(id)) films.update(id, Film(id, titles[random() % 8], 100));
        for (const auto& query : queries)
        {
            std::vector<EntityId> expected;
            for (const Film& film : films)
                assert(film.normalizedTitle() == text_normalization::normalize(film.title()));
            LinkedList<const Film*> sequential;
            films.searchTitleSequential(query, sequential);
            for (const Film* film : sequential) expected.push_back(film->id());
            std::sort(expected.begin(), expected.end());
            assert(search(films, query) == expected);
        }
    }
}

int main()
{
    testRequirementsAndUnicode();
    testCrud();
    testDifferential();
    std::cout << "Film search checks passed\n";
}
