#pragma once

#include "DataStructures/LinkedList.h"
#include "entities/EntityId.h"

template <typename T>
class IRepository
{
public:
    virtual ~IRepository() = default;

    // IDs are unique; nextId() returns an unused positive ID or throws on exhaustion.
    // Stored addresses stay valid across insert/update, until erase/replacement.
    // A failed write leaves stored records unchanged; erase does not allocate.
    virtual EntityId nextId() const = 0;
    virtual const T* findById(EntityId id) const = 0;
    virtual bool insert(const T& value) = 0;
    virtual bool update(EntityId id, const T& value) = 0;
    virtual bool erase(EntityId id) = 0;

    // Output is a snapshot of borrowed pointers, not copies of stored entities.
    // Failure leaves result unchanged. Erasing/replacing records invalidates pointers.
    virtual void list(LinkedList<const T*>& result) const = 0;
};
