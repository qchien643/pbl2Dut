#pragma once

#include "DataStructures/LinkedList.h"
#include "entities/Auditorium.h"
#include "entities/Seat.h"

// Persist a room and its complete new seat layout as one RAM operation.
// The service checks business rules; failure must leave repositories unchanged.
class IRoomStorage
{
public:
    virtual ~IRoomStorage() = default;
    virtual void add(const Auditorium& room, const LinkedList<Seat>& seats) = 0;
    virtual void update(const Auditorium& room, const LinkedList<Seat>& seats) = 0;
};
