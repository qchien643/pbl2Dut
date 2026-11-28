#include "repositories/FilmRepository.h"
#include "helper/TextNormalization.h"
#include "search/SequentialSearch.h"
#include "search/TextSearch.h"

bool FilmRepository::insert(const Film& value)
{
    if (findById(value.id())) return false;
    return write(value, false);
}

bool FilmRepository::update(EntityId id, const Film& value)
{
    if (id != value.id()) throw std::invalid_argument("Cannot change an entity ID");
    if (!findById(id)) return false;
    return write(value, true);
}

bool FilmRepository::write(const Film& value, bool updating)
{
    titleIndex_.write(value, value.normalizedTitle(), [&]() -> const Film* {
        const bool success = updating ? Repository<Film>::update(value.id(), value)
                                      : Repository<Film>::insert(value);
        if (!success) throw std::logic_error("Film changed during index preparation");
        return findById(value.id());
    });
    return true;
}

bool FilmRepository::erase(EntityId id)
{
    const Film* record = findById(id);
    if (!record) return false;
    titleIndex_.erase(*record);
    return Repository<Film>::erase(id);
}

void FilmRepository::replaceWith(FilmRepository& replacement) noexcept
{
    if (this == &replacement) return;
    Repository<Film>::replaceWith(replacement);
    titleIndex_.swap(replacement.titleIndex_);
}

void FilmRepository::searchTitle(const std::string& query,
                                LinkedList<const Film*>& result) const
{
    if (query.find_first_not_of(" \t\r\n\f\v") == std::string::npos)
    { list(result); return; }
    const auto words = text_search::queryWords(text_normalization::normalize(query));
    LinkedList<const Film*> matches;
    for (const Film* film : titleIndex_.candidates(words))
        if (text_search::matches(film->normalizedTitle(), words)) matches.pushBack(film);
    result.swap(matches);
}

void FilmRepository::searchTitleSequential(const std::string& query,
                                           LinkedList<const Film*>& result) const
{
    if (query.find_first_not_of(" \t\r\n\f\v") == std::string::npos)
    { list(result); return; }
    const auto words = text_search::queryWords(text_normalization::normalize(query));
    search::sequentialSearch(*this, [&](const Film& film) {
        return text_search::matches(film.normalizedTitle(), words);
    }, result);
}
