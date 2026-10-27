#include "TicketRepository.h"

bool TicketRepository::empty() const
{
    return this->records_.empty();
}

std::size_t TicketRepository::size() const
{
    return this->records_.size();
}

bool TicketRepository::isDirty() const
{
    return this->records_.isDirty();
}

void TicketRepository::markSaved()
{
    this->records_.markSaved();
}

EntityId TicketRepository::nextId() const
{
    return this->records_.nextId();
}

const Ticket* TicketRepository::findById(EntityId id) const
{
    return this->records_.findById(id);
}

bool TicketRepository::insert(const Ticket& ticket)
{
    return this->records_.insert(ticket);
}

LinkedList<Ticket>::ConstIterator TicketRepository::begin() const
{
    return this->records_.begin();
}

LinkedList<Ticket>::ConstIterator TicketRepository::end() const
{
    return this->records_.end();
}

void TicketRepository::relinkSeats(const SeatRepository& seats) noexcept
{
    // ponytail: services preserve seat IDs; add checked relinking if direct repository edits become supported.
    for (const Ticket& ticket : *this)
    {
        Ticket* stored = this->records_.findMutableById(ticket.id());
        stored->setSeat(*seats.findById(ticket.seatId()));
    }
}
