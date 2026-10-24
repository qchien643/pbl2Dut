#pragma once

#include "cinema/structer/DoubleLinkedList.h"

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>

namespace cinema {

template <typename Key>
struct DefaultHash {
    std::size_t operator()(const Key& key) const noexcept {
        return static_cast<std::size_t>(key);
    }
};

template <typename Key>
struct DefaultKeyEqual {
    bool operator()(const Key& left, const Key& right) const {
        return left == right;
    }
};

template <typename Key, typename Value, typename Hash = DefaultHash<Key>,
          typename KeyEqual = DefaultKeyEqual<Key>>
class HashTable {
private:
    struct Entry {
        Key key;
        Value value;
    };

public:
    explicit HashTable(std::size_t bucketCount = 8)
        : bucketCount_(bucketCount == 0 ? 8 : bucketCount),
          buckets_(new DoubleLinkedList<Entry>[bucketCount_]) {}

    HashTable(const HashTable& other)
        : bucketCount_(other.bucketCount_),
          buckets_(nullptr), hash_(other.hash_), equal_(other.equal_) {
        buckets_ = new DoubleLinkedList<Entry>[bucketCount_];
        try {
            other.forEach([this](const Key& key, const Value& value) {
                buckets_[bucketIndex(key)].append(Entry{key, value});
                ++size_;
            });
        } catch (...) {
            delete[] buckets_;
            buckets_ = nullptr;
            throw;
        }
    }

    HashTable& operator=(const HashTable& other) {
        if (this != &other) {
            HashTable copy(other);
            swap(copy);
        }
        return *this;
    }

    HashTable(HashTable&& other)
        : bucketCount_(8), buckets_(new DoubleLinkedList<Entry>[8]) {
        swap(other);
    }

    HashTable& operator=(HashTable&& other) {
        if (this != &other) {
            HashTable moved(std::move(other));
            swap(moved);
        }
        return *this;
    }

    ~HashTable() {
        delete[] buckets_;
    }

    bool insert(const Key& key, const Value& value) {
        if (find(key)) {
            return false;
        }
        ensureCapacity(size_ + 1);
        buckets_[bucketIndex(key)].append(Entry{key, value});
        ++size_;
        return true;
    }

    bool insertOrAssign(const Key& key, const Value& value) {
        if (Value* existing = find(key)) {
            *existing = value;
            return false;
        }
        return insert(key, value);
    }

    Value* find(const Key& key) {
        Entry* entry = buckets_[bucketIndex(key)].findIf(
            [this, &key](const Entry& candidate) { return equal_(candidate.key, key); });
        return entry ? &entry->value : nullptr;
    }

    const Value* find(const Key& key) const {
        const Entry* entry = buckets_[bucketIndex(key)].findIf(
            [this, &key](const Entry& candidate) { return equal_(candidate.key, key); });
        return entry ? &entry->value : nullptr;
    }

    bool contains(const Key& key) const {
        return find(key) != nullptr;
    }

    bool erase(const Key& key) {
        const bool erased = buckets_[bucketIndex(key)].eraseFirst(
            [this, &key](const Entry& candidate) { return equal_(candidate.key, key); });
        if (erased) {
            --size_;
        }
        return erased;
    }

    void clear() {
        for (std::size_t index = 0; index < bucketCount_; ++index) {
            buckets_[index].clear();
        }
        size_ = 0;
    }

    template <typename Function>
    void forEach(Function function) const {
        for (std::size_t index = 0; index < bucketCount_; ++index) {
            buckets_[index].forEach([&function](const Entry& entry) {
                function(entry.key, entry.value);
            });
        }
    }

    std::size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }

    void swap(HashTable& other) noexcept {
        const std::size_t savedBucketCount = bucketCount_;
        bucketCount_ = other.bucketCount_;
        other.bucketCount_ = savedBucketCount;

        DoubleLinkedList<Entry>* savedBuckets = buckets_;
        buckets_ = other.buckets_;
        other.buckets_ = savedBuckets;

        const std::size_t savedSize = size_;
        size_ = other.size_;
        other.size_ = savedSize;

        Hash savedHash(std::move(hash_));
        hash_ = std::move(other.hash_);
        other.hash_ = std::move(savedHash);

        KeyEqual savedEqual(std::move(equal_));
        equal_ = std::move(other.equal_);
        other.equal_ = std::move(savedEqual);
    }

private:
    std::size_t bucketIndex(const Key& key) const {
        return hash_(key) % bucketCount_;
    }

    void ensureCapacity(std::size_t desiredSize) {
        if (desiredSize <= bucketCount_ - bucketCount_ / 4) {
            return;
        }
        if (bucketCount_ > std::numeric_limits<std::size_t>::max() / 2) {
            throw std::length_error("HashTable capacity overflow");
        }

        const std::size_t newCount = bucketCount_ * 2;
        DoubleLinkedList<Entry>* newBuckets = new DoubleLinkedList<Entry>[newCount];
        try {
            forEach([this, newBuckets, newCount](const Key& key, const Value& value) {
                const std::size_t index = hash_(key) % newCount;
                newBuckets[index].append(Entry{key, value});
            });
        } catch (...) {
            delete[] newBuckets;
            throw;
        }

        delete[] buckets_;
        buckets_ = newBuckets;
        bucketCount_ = newCount;
    }

    std::size_t bucketCount_;
    DoubleLinkedList<Entry>* buckets_;
    std::size_t size_ = 0;
    Hash hash_{};
    KeyEqual equal_{};
};

} // namespace cinema
