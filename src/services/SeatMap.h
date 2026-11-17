#pragma once

#include "DataStructures/Matrix2D.h"
#include "entities/Auditorium.h"
#include "entities/Seat.h"
#include "repositories/IRepository.h"

#include <memory>

struct SeatStatus {
    EntityId seatId = 0;
    bool sold = false;
};

using SeatMap = Matrix2D<SeatStatus>;

std::unique_ptr<SeatMap> makeSeatMap(const Auditorium& room, const IRepository<Seat>& seats);
