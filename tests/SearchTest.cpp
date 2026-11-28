#ifdef NDEBUG
#undef NDEBUG
#endif

#include "search/SequentialSearch.h"
#include "search/TextSearch.h"
#include "search/TrigramIndex.h"
#include "entities/Auditorium.h"

#include <cassert>
#include <stdexcept>
#include <utility>
#include <vector>

// Test-only model: two text fields with independent indexes, not a new entity.
struct Document {
    EntityId identifier;
    std::u32string name;
    std::u32string description;
    EntityId id() const { return identifier; }
};

void testIndependentColumns()
{
    Document first{40, U"nguyen trai", U"mat biec"};
    Document second{2, U"mat biec", U"nguyen trai"};
    TrigramIndex<Document> names;
    TrigramIndex<Document> descriptions;
    for (Document* record : {&first, &second})
    {
        names.write(*record, record->name, [&] { return record; });
        descriptions.write(*record, record->description, [&] { return record; });
    }
    const auto query = text_search::queryWords(U"nguyen trai");
    assert(names.candidates(query) == std::vector<const Document*>{&first});
    assert(descriptions.candidates(query) == std::vector<const Document*>{&second});

    // Different storage objects can share an ID across independent indexes.
    Document other{40, U"nguoi nhen", U""};
    TrigramIndex<Document> separate;
    separate.write(other, other.name, [&] { return &other; });
    assert(separate.candidates(query).empty());
    assert(names.candidates(query) == std::vector<const Document*>{&first});

    // Callback failure must leave postings and field metadata unchanged.
    Document replacement{40, U"nguoi nhen", U"mat biec"};
    bool failed = false;
    try {
        names.write(replacement, replacement.name, [&]() -> const Document* {
            throw std::runtime_error("Storage write failed");
        });
    } catch (const std::runtime_error&) { failed = true; }
    assert(failed && names.candidates(query) == std::vector<const Document*>{&first});
    const auto spider = text_search::queryWords(U"nguoi nhen");
    assert(names.candidates(spider).empty());

    names.write(replacement, replacement.name, [&] {
        std::swap(first, replacement);
        return &first;
    });
    assert(names.candidates(query).empty());
    assert(names.candidates(spider) == std::vector<const Document*>{&first});
    assert(descriptions.candidates(query) == std::vector<const Document*>{&second});
    names.erase(first);
    assert(names.candidates(spider).empty());
    names.erase(second);
    assert(names.candidates(text_search::queryWords(U"mat biec")).empty());

    TrigramIndex<Document> empty;
    descriptions.swap(empty);
    assert(descriptions.candidates(query).empty());
    assert(empty.candidates(query) == std::vector<const Document*>{&second});

    // Index returns candidates, not guaranteed matches.
    Document noise{1, U"nguxyz abc def", U""};
    TrigramIndex<Document> candidates;
    candidates.write(noise, noise.name, [&] { return &noise; });
    const auto typo = text_search::queryWords(U"nguyn");
    assert(candidates.candidates(typo) == std::vector<const Document*>{&noise});
    assert(!text_search::matches(noise.name, typo));

    // Short fields have no postings but still need metadata for later updates/erase.
    Document shortText{7, U"ab", U""};
    TrigramIndex<Document> shortIndex;
    shortIndex.write(shortText, shortText.name, [&] { return &shortText; });
    assert(shortIndex.candidates(query).empty());
    Document longText{7, U"nguyen trai", U""};
    shortIndex.write(longText, longText.name, [&] {
        std::swap(shortText, longText);
        return &shortText;
    });
    assert(shortIndex.candidates(query) == std::vector<const Document*>{&shortText});
    shortIndex.write(longText, longText.name, [&] {
        std::swap(shortText, longText);
        return &shortText;
    });
    assert(shortIndex.candidates(query).empty());
    shortIndex.erase(shortText);
}

void testScanDifferentColumnsAndTypes()
{
    std::vector<Document> documents{{1, U"nguyen trai", U"mat biec"},
                                    {2, U"mat biec", U"nguyen trai"}};
    const auto query = text_search::queryWords(U"nguyen trai");
    LinkedList<const Document*> found;
    search::sequentialSearch(documents, [&](const Document& record) {
        return text_search::matches(record.name, query);
    }, found);
    assert(found.size() == 1 && *found.begin() == &documents[0]);
    search::sequentialSearch(documents, [&](const Document& record) {
        return text_search::matches(record.description, query);
    }, found);
    assert(found.size() == 1 && *found.begin() == &documents[1]);
    search::sequentialSearch(documents, [](const Document& record) {
        return record.id() == 1;
    }, found);
    assert(found.size() == 1 && *found.begin() == &documents[0]);

    bool failed = false;
    try {
        search::sequentialSearch(documents, [](const Document& record) {
            if (record.id() == 2) throw std::runtime_error("Predicate failed");
            return true;
        }, found);
    } catch (const std::runtime_error&) { failed = true; }
    assert(failed && found.size() == 1 && *found.begin() == &documents[0]);

    LinkedList<Auditorium> rooms;
    rooms.pushBack(Auditorium(1, "Small", 2, 3));
    const Auditorium& large = rooms.pushBack(Auditorium(2, "Large", 10, 10));
    LinkedList<const Auditorium*> selected;
    search::sequentialSearch(rooms, [](const Auditorium& room) {
        return room.capacity() >= 50;
    }, selected);
    assert(selected.size() == 1 && *selected.begin() == &large);
    LinkedList<Auditorium> empty;
    search::sequentialSearch(empty, [](const Auditorium&) { return true; }, selected);
    assert(selected.empty());
}

int main()
{
    testIndependentColumns();
    testScanDifferentColumnsAndTypes();
}
