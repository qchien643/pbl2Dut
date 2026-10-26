#include "services/SeatMap.h"

#include <cstdint>
#include <limits>
#include <stdexcept>

std::unique_ptr<SeatMap> makeSeatMap(const Auditorium& room, const SeatRepository& seats)
{
    const std::size_t maximum = std::numeric_limits<std::size_t>::max();
    if (static_cast<std::uint64_t>(room.rowCount()) > maximum
        || static_cast<std::uint64_t>(room.seatsPerRow()) > maximum)
    {
        throw std::length_error("Room dimensions exceed address space");
    }
    const std::size_t rows = static_cast<std::size_t>(room.rowCount());
    const std::size_t columns = static_cast<std::size_t>(room.seatsPerRow());
    std::unique_ptr<SeatMap> result = std::make_unique<SeatMap>(rows, columns);
    std::size_t count = 0;
    for (const Seat& seat : seats)
    {
        if (seat.auditoriumId() != room.id())
        {
            continue;
        }
        if (seat.rowNumber() > room.rowCount() || seat.seatNumber() > room.seatsPerRow())
        {
            throw std::invalid_argument("Seat outside room");
        }
        const std::size_t row = static_cast<std::size_t>(seat.rowNumber() - 1);
        const std::size_t column = static_cast<std::size_t>(seat.seatNumber() - 1);
        SeatStatus& cell = result->at(row, column);
        if (cell.seatId != 0)
        {
            throw std::invalid_argument("Duplicate seat position");
        }
        cell.seatId = seat.id();
        ++count;
    }
    if (count != result->size())
    {
        throw std::invalid_argument("Incomplete room seats");
    }
    return result;
}
