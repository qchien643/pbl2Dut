#pragma once

#include "entities/EntityId.h"

#include <cstdint>
#include <string>

class Auditorium
{
public:
    Auditorium(EntityId id, std::string name, std::int64_t rowCount, std::int64_t seatsPerRow);

    EntityId id() const;
    const std::string& name() const;
    std::int64_t rowCount() const;
    std::int64_t seatsPerRow() const;
    std::int64_t capacity() const;

private:
    EntityId id_;
    std::string name_;
    std::int64_t rowCount_;
    std::int64_t seatsPerRow_;
};
