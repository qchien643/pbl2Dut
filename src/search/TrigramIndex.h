#pragma once

#include "DataStructures/HashTable.h"
#include "entities/EntityId.h"
#include "search/TextSearch.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <vector>

// One instance indexes one normalized text field.
// T supplies id(); objects have unique IDs and stable addresses until erase.
// This index borrows objects; it never owns or copies the repository's records.
template <typename T>
class TrigramIndex
{
public:
    // Prepare index changes before writing storage. store() must either throw
    // without changing storage, or return the stored object's stable address.
    template <typename Store>
    void write(const T& value, const std::u32string& normalizedText, Store store)
    {
        const EntityId id = value.id();
        std::vector<std::string> keys = text_search::makeTrigrams(normalizedText);
        const auto* previous = recordTrigrams_.find(id);
        const std::vector<std::string> oldKeys = previous ? *previous : std::vector<std::string>{};
        if (previous && keys == oldKeys) { store(); return; }

        struct Change {
            std::string key;
            std::vector<const T*> records;
            bool created = false;
        };
        std::vector<std::string> changedKeys;
        std::set_symmetric_difference(keys.begin(), keys.end(), oldKeys.begin(), oldKeys.end(),
                                     std::back_inserter(changedKeys));
        std::vector<Change> changes;
        for (const auto& key : changedKeys)
        {
            const auto* stored = postings_.find(key);
            Change change{key, stored ? *stored : std::vector<const T*>{}};
            const auto position = std::lower_bound(change.records.begin(), change.records.end(), &value, recordBefore);
            // Temporary pointer only; commit binds it to the stored object.
            if (std::binary_search(keys.begin(), keys.end(), key)) change.records.insert(position, &value);
            else if (position != change.records.end() && (*position)->id() == id) change.records.erase(position);
            changes.push_back(std::move(change));
        }

        const T* record;
        bool metadataCreated = false;
        try
        {
            // Prepare empty slots before changing the entity; rehash invalidates value pointers.
            for (auto& change : changes)
                if (!postings_.find(change.key))
                    change.created = postings_.insert(change.key, {});
            if (!previous) metadataCreated = recordTrigrams_.insert(id, {});
            record = store();
        }
        catch (...)
        {
            for (const auto& change : changes)
                if (change.created) postings_.erase(change.key);
            if (metadataCreated) recordTrigrams_.erase(id);
            throw;
        }
        // Commit only non-allocating operations after the entity write succeeds.
        for (auto& change : changes)
        {
            const auto position = std::lower_bound(change.records.begin(), change.records.end(), record, recordBefore);
            if (position != change.records.end() && (*position)->id() == id) *position = record;
            postings_.find(change.key)->swap(change.records);
            if (postings_.find(change.key)->empty()) postings_.erase(change.key);
        }
        recordTrigrams_.find(id)->swap(keys);
    }

    // Remove pointers before the repository destroys the object; no allocations.
    void erase(const T& value)
    {
        const EntityId id = value.id();
        const T* record = &value;
        const auto* keys = recordTrigrams_.find(id);
        if (!keys) throw std::logic_error("Missing trigram index metadata");
        for (const auto& key : *keys)
        {
            auto* records = postings_.find(key);
            const auto position = std::lower_bound(records->begin(), records->end(), record, recordBefore);
            if (position != records->end() && *position == record) records->erase(position);
            if (records->empty()) postings_.erase(key);
        }
        recordTrigrams_.erase(id);
    }

    // Candidates only: the caller must verify the selected field with matches().
    std::vector<const T*> candidates(const std::vector<std::u32string>& words) const
    {
        std::vector<const T*> candidates;
        bool first = true;
        for (const auto& word : words)
        {
            std::vector<const T*> records;
            for (const auto& gram : text_search::makeTrigrams(word))
            {
                const auto* posting = postings_.find(gram);
                if (!posting) continue;
                std::vector<const T*> joined;
                std::set_union(records.begin(), records.end(), posting->begin(), posting->end(),
                               std::back_inserter(joined), recordBefore);
                records.swap(joined);
            }
            if (first) { candidates.swap(records); first = false; }
            else
            {
                std::vector<const T*> intersection;
                std::set_intersection(candidates.begin(), candidates.end(), records.begin(), records.end(),
                                      std::back_inserter(intersection), recordBefore);
                candidates.swap(intersection);
            }
            if (candidates.empty()) break;
        }
        return candidates;
    }

    void swap(TrigramIndex& other) noexcept
    {
        postings_.swap(other.postings_);
        recordTrigrams_.swap(other.recordTrigrams_);
    }

private:
    static bool recordBefore(const T* left, const T* right)
    {
        return left->id() < right->id();
    }

    HashTable<std::string, std::vector<const T*>> postings_;
    HashTable<EntityId, std::vector<std::string>> recordTrigrams_;
};
