#pragma once

#include "entities/Ticket.h"
#include "repositories/Repository.h"
#include "repositories/SeatRepository.h"

class TicketRepository final
{
public:
    using value_type = Ticket;

    bool empty() const;
    std::size_t size() const;
    bool isDirty() const;
    void markSaved();
    EntityId nextId() const;
    const Ticket* findById(EntityId id) const;
    bool insert(const Ticket& ticket);

    LinkedList<Ticket>::ConstIterator begin() const;
    LinkedList<Ticket>::ConstIterator end() const;

private:
    friend class ApplicationData;
    friend class CatalogService;
    void relinkSeats(const SeatRepository& seats) noexcept;
    Repository<Ticket> records_;
};

