#pragma once

#include "DataStructures/Matrix2D.h"
#include "DataStructures/LinkedList.h"
#include "entities/Seat.h"

#include <cstdint>
#include <memory>

struct SeatStatus {
    EntityId seatId = 0;
    bool sold = false;
};

using SeatMap = Matrix2D<SeatStatus>;

SeatStatus& seatMapCell(SeatMap& map, const Seat& seat);

// Empty positions remain seatId=0; selecting the room and requiring a full layout belong to services.
std::unique_ptr<SeatMap> makeSeatMap(std::int64_t rowCount, std::int64_t seatsPerRow,
                                    const LinkedList<const Seat*>& seats);
