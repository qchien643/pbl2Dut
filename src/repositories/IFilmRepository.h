#pragma once

#include "entities/Film.h"
#include "repositories/IRepository.h"

#include <string>

class IFilmRepository : public virtual IRepository<Film>
{
public:
    ~IFilmRepository() override = default;
    // Borrowed read-only pointers. A failed query leaves result unchanged.
    virtual void searchTitle(const std::string& query,
                             LinkedList<const Film*>& result) const = 0;
};
