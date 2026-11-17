#pragma once

#include "DataStructures/LinkedList.h"
#include "entities/Ticket.h"

// Sold tickets deliberately have no update/erase contract.
class ITicketRepository
{
public:
    virtual ~ITicketRepository() = default;
    virtual EntityId nextId() const = 0;
    virtual const Ticket* findById(EntityId id) const = 0;
    virtual bool insert(const Ticket& ticket) = 0;
    virtual void list(LinkedList<const Ticket*>& result) const = 0;
};
