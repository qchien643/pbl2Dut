#include "helper/SeatMap.h"

#include <cstdint>
#include <limits>
#include <stdexcept>

SeatStatus& seatMapCell(SeatMap& map, const Seat& seat)
{
    if (static_cast<std::uint64_t>(seat.rowNumber()) > map.rows()
        || static_cast<std::uint64_t>(seat.seatNumber()) > map.columns())
    {
        throw std::invalid_argument("Seat outside matrix");
    }
    const std::size_t row = static_cast<std::size_t>(seat.rowNumber() - 1);
    const std::size_t column = static_cast<std::size_t>(seat.seatNumber() - 1);
    return map.at(row, column);
}

std::unique_ptr<SeatMap> makeSeatMap(std::int64_t rowCount, std::int64_t seatsPerRow,
                                    const LinkedList<const Seat*>& seats)
{
    if (rowCount <= 0 || seatsPerRow <= 0)
    {
        throw std::invalid_argument("Matrix dimensions must be positive");
    }
    const std::size_t maximum = std::numeric_limits<std::size_t>::max();
    if (static_cast<std::uint64_t>(rowCount) > maximum
        || static_cast<std::uint64_t>(seatsPerRow) > maximum)
    {
        throw std::length_error("Room dimensions exceed address space");
    }
    const std::size_t rows = static_cast<std::size_t>(rowCount);
    const std::size_t columns = static_cast<std::size_t>(seatsPerRow);
    std::unique_ptr<SeatMap> result = std::make_unique<SeatMap>(rows, columns);
    for (const Seat* record : seats)
    {
        if (record == nullptr)
        {
            throw std::invalid_argument("Seat pointer must not be null");
        }
        const Seat& seat = *record;
        SeatStatus& cell = seatMapCell(*result, seat);
        if (cell.seatId != 0)
        {
            throw std::invalid_argument("Duplicate seat position");
        }
        cell.seatId = seat.id();
    }
    return result;
}
