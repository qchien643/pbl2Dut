#pragma once

#include "LinkedList.h"

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

template <typename Key>
struct DefaultHash {
    std::size_t operator()(const Key& key) const {
        static_assert(std::is_integral_v<Key>, "Provide a hasher for non-integral keys");
        return static_cast<std::size_t>(key);
    }
};

template <>
struct DefaultHash<std::string> {
    std::size_t operator()(const std::string& key) const {
        std::size_t hash = 0;
        for (unsigned char character : key) {
            hash = hash * 31 + character;
        }
        return hash;
    }
};

template <typename Key, typename Value, typename Hasher = DefaultHash<Key>>
class HashTable {
public:
    explicit HashTable(std::size_t bucketCount = 8, const Hasher& hasher = Hasher{});

    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;
    HashTable(HashTable&&) = delete;
    HashTable& operator=(HashTable&&) = delete;

    ~HashTable();

    void swap(HashTable& other) noexcept(std::is_nothrow_swappable_v<Hasher>);

    bool empty() const;
    std::size_t size() const;
    std::size_t bucketCount() const;

    Value* find(const Key& key);
    const Value* find(const Key& key) const;
    bool insert(const Key& key, const Value& value);
    bool erase(const Key& key);
    void rehash(std::size_t newBucketCount);
    void clear() noexcept;

private:
    struct Entry {
        Key key;
        Value value;
    };

    using Bucket = LinkedList<Entry>;

    static Bucket* allocateBuckets(std::size_t count);

    Hasher hasher_;
    std::size_t bucketCount_;
    Bucket* buckets_;
    std::size_t size_ = 0;
};

template <typename Key, typename Value, typename Hasher>
HashTable<Key, Value, Hasher>::HashTable(std::size_t bucketCount, const Hasher& hasher)
    : hasher_(hasher), bucketCount_(bucketCount), buckets_(allocateBuckets(bucketCount))
{
}

template <typename Key, typename Value, typename Hasher>
HashTable<Key, Value, Hasher>::~HashTable()
{
    delete[] buckets_;
}

template <typename Key, typename Value, typename Hasher>
void HashTable<Key, Value, Hasher>::swap(HashTable& other) noexcept(std::is_nothrow_swappable_v<Hasher>)
{
    using std::swap;
    swap(hasher_, other.hasher_);
    swap(bucketCount_, other.bucketCount_);
    swap(buckets_, other.buckets_);
    swap(size_, other.size_);
}

template <typename Key, typename Value, typename Hasher>
bool HashTable<Key, Value, Hasher>::empty() const
{
    return size_ == 0;
}

template <typename Key, typename Value, typename Hasher>
std::size_t HashTable<Key, Value, Hasher>::size() const
{
    return size_;
}

template <typename Key, typename Value, typename Hasher>
std::size_t HashTable<Key, Value, Hasher>::bucketCount() const
{
    return bucketCount_;
}

template <typename Key, typename Value, typename Hasher>
Value* HashTable<Key, Value, Hasher>::find(const Key& key)
{
    const std::size_t index = hasher_(key) % bucketCount_;
    for (Entry& entry : buckets_[index]) {
        if (entry.key == key) {
            return &entry.value;
        }
    }
    return nullptr;
}

template <typename Key, typename Value, typename Hasher>
const Value* HashTable<Key, Value, Hasher>::find(const Key& key) const
{
    const Bucket& bucket = buckets_[hasher_(key) % bucketCount_];
    for (const Entry& entry : bucket) {
        if (entry.key == key) {
            return &entry.value;
        }
    }
    return nullptr;
}

template <typename Key, typename Value, typename Hasher>
bool HashTable<Key, Value, Hasher>::insert(const Key& key, const Value& value)
{
    if (find(key) != nullptr) {
        return false;
    }
    Entry candidate{key, value};
    if (size_ == bucketCount_) {
        if (bucketCount_ > std::numeric_limits<std::size_t>::max() / 2) {
            throw std::length_error("Hash table capacity overflow");
        }
        rehash(bucketCount_ * 2);
    }
    buckets_[hasher_(candidate.key) % bucketCount_].pushBack(candidate);
    ++size_;
    return true;
}

template <typename Key, typename Value, typename Hasher>
bool HashTable<Key, Value, Hasher>::erase(const Key& key)
{
    Bucket& bucket = buckets_[hasher_(key) % bucketCount_];
    for (const Entry& entry : bucket) {
        if (entry.key == key) {
            bucket.erase(&entry);
            --size_;
            return true;
        }
    }
    return false;
}

template <typename Key, typename Value, typename Hasher>
void HashTable<Key, Value, Hasher>::rehash(std::size_t newBucketCount)
{
    if (newBucketCount < size_) {
        throw std::invalid_argument("Bucket count cannot be smaller than size");
    }
    if (newBucketCount == bucketCount_) {
        return;
    }
    Bucket* replacement = allocateBuckets(newBucketCount);
    try {
        for (std::size_t index = 0; index < bucketCount_; ++index) {
            for (const Entry& entry : buckets_[index]) {
                const std::size_t newIndex = hasher_(entry.key) % newBucketCount;
                const Entry copy{entry.key, entry.value};
                replacement[newIndex].pushBack(copy);
            }
        }
    } catch (...) {
        delete[] replacement;
        throw;
    }
    delete[] buckets_;
    buckets_ = replacement;
    bucketCount_ = newBucketCount;
}

template <typename Key, typename Value, typename Hasher>
void HashTable<Key, Value, Hasher>::clear() noexcept
{
    for (std::size_t index = 0; index < bucketCount_; ++index) {
        buckets_[index].clear();
    }
    size_ = 0;
}

template <typename Key, typename Value, typename Hasher>
typename HashTable<Key, Value, Hasher>::Bucket* HashTable<Key, Value, Hasher>::allocateBuckets(std::size_t count)
{
    if (count == 0) {
        throw std::invalid_argument("Bucket count must be positive");
    }
    if (count > std::numeric_limits<std::size_t>::max() / sizeof(Bucket)) {
        throw std::length_error("Hash table allocation size overflow");
    }
    return new Bucket[count];
}
