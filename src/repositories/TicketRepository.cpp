#include "TicketRepository.h"

void TicketRepository::relinkSeats(const Repository<Seat>& seats) noexcept
{
    // ponytail: services preserve seat IDs; add checked relinking if direct repository edits become supported.
    for (const Ticket& ticket : *this)
    {
        Ticket* stored = this->findMutableById(ticket.id());
        stored->setSeat(*seats.findById(ticket.seatId()));
    }
}
