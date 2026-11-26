#pragma once

#include "entities/Seat.h"
#include "entities/Ticket.h"
#include "repositories/Repository.h"

class TicketRepository final : public Repository<Ticket>
{
private:
    friend class RoomStorage;
    void relinkSeats(const Repository<Seat>& seats) noexcept;
};

// Repository cung cấp CRUD chung; TicketService không cung cấp sửa/xóa vé đã bán.
