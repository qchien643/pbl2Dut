#pragma once

#include "entities/Film.h"
#include "repositories/Repository.h"
#include "repositories/IFilmRepository.h"
#include "search/TrigramIndex.h"

class FilmRepository final : public Repository<Film>, public IFilmRepository
{
public:
    bool insert(const Film& value) override;
    bool update(EntityId id, const Film& value) override;
    bool erase(EntityId id) override;
    void searchTitle(const std::string& query,
                     LinkedList<const Film*>& result) const override;
    // Reference scan for tests/benchmark; uses the same matcher as indexed search.
    void searchTitleSequential(const std::string& query,
                               LinkedList<const Film*>& result) const;
    void replaceWith(FilmRepository& replacement) noexcept;

private:
    bool write(const Film& value, bool updating);
    TrigramIndex<Film> titleIndex_;
};
