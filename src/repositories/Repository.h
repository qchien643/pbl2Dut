#pragma once

#include "DataStructures/HashTable.h"
#include "DataStructures/LinkedList.h"
#include "entities/EntityId.h"
#include "entities/EntityValidation.h"

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

template <typename T>
class Repository
{
public:
    using value_type = T;

    Repository() = default;
    Repository(const Repository&) = delete;
    Repository& operator=(const Repository&) = delete;
    Repository(Repository&&) = delete;
    Repository& operator=(Repository&&) = delete;

    bool empty() const;
    std::size_t size() const;
    bool isDirty() const;
    void markSaved();

    EntityId nextId() const;
    const T* findById(EntityId id) const;
    bool insert(const T& value);
    bool update(EntityId id, const T& value);
    bool erase(EntityId id);

    typename LinkedList<T>::ConstIterator begin() const;
    typename LinkedList<T>::ConstIterator end() const;
    void replaceWith(Repository& replacement) noexcept;

private:
    LinkedList<T> records_;
    HashTable<EntityId, T*> index_;
    EntityId highestId_ = 0;
    bool dirty_ = false;
};

template <typename T>
bool Repository<T>::empty() const
{
    return this->records_.empty();
}

template <typename T>
std::size_t Repository<T>::size() const
{
    return this->records_.size();
}

template <typename T>
bool Repository<T>::isDirty() const
{
    return this->dirty_;
}

template <typename T>
void Repository<T>::markSaved()
{
    this->dirty_ = false;
}

template <typename T>
EntityId Repository<T>::nextId() const
{
    if (this->highestId_ == std::numeric_limits<EntityId>::max())
    {
        throw std::overflow_error("Entity ID exhausted");
    }
    return this->highestId_ + 1;
}

template <typename T>
const T* Repository<T>::findById(EntityId id) const
{
    // The hash table stores T*, so find() returns the address of that pointer.
    T* const* indexed = this->index_.find(id);
    if (indexed == nullptr)
    {
        return nullptr;
    }
    const T* record = *indexed;
    return record;
}

template <typename T>
bool Repository<T>::insert(const T& value)
{
    const EntityId id = value.id();
    entity_detail::positive(id, "Entity ID");
    if (this->index_.find(id) != nullptr)
    {
        return false;
    }

    T& record = this->records_.pushBack(value);
    try
    {
        if (!this->index_.insert(id, &record))
        {
            this->records_.erase(&record);
            return false;
        }
    }
    catch (...)
    {
        // Undo the list insertion if the index cannot be updated.
        this->records_.erase(&record);
        throw;
    }

    if (id > this->highestId_)
    {
        this->highestId_ = id;
    }
    this->dirty_ = true;
    return true;
}

template <typename T>
bool Repository<T>::update(EntityId id, const T& value)
{
    static_assert(std::is_nothrow_swappable_v<T>, "Repository update requires noexcept swap");
    if (id != value.id())
    {
        throw std::invalid_argument("Cannot change an entity ID");
    }
    T** indexed = this->index_.find(id);
    if (indexed == nullptr)
    {
        return false;
    }
    T* record = *indexed;

    // Copy first: a failed copy leaves the stored object unchanged.
    T replacement(value);
    using std::swap;
    swap(*record, replacement);
    this->dirty_ = true;
    return true;
}

template <typename T>
bool Repository<T>::erase(EntityId id)
{
    const T* record = this->findById(id);
    if (record == nullptr)
    {
        return false;
    }

    this->index_.erase(id);
    this->records_.erase(record);
    this->dirty_ = true;
    return true;
}

template <typename T>
typename LinkedList<T>::ConstIterator Repository<T>::begin() const
{
    const LinkedList<T>& records = this->records_;
    return records.begin();
}

template <typename T>
typename LinkedList<T>::ConstIterator Repository<T>::end() const
{
    const LinkedList<T>& records = this->records_;
    return records.end();
}

template <typename T>
void Repository<T>::replaceWith(Repository& replacement) noexcept
{
    if (this == &replacement)
    {
        return;
    }

    // Move the list and its index together so every pointer stays valid.
    const EntityId previousHighest = this->highestId_;
    this->records_.swap(replacement.records_);
    this->index_.swap(replacement.index_);
    std::swap(this->highestId_, replacement.highestId_);
    std::swap(this->dirty_, replacement.dirty_);
    if (this->highestId_ < previousHighest)
    {
        this->highestId_ = previousHighest;
    }
    this->dirty_ = true;
}
