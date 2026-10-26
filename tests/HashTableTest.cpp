#ifdef NDEBUG
#undef NDEBUG
#endif

#include "DataStructures/HashTable.h"
#include "DataStructures/Matrix2D.h"

#include <cassert>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

struct ConstantHash {
    std::size_t operator()(int) const noexcept {
        return 0;
    }
};

struct FailingHash {
    inline static int failingKey = -1;

    std::size_t operator()(int key) const {
        if (key == failingKey) {
            throw std::runtime_error("hash failed");
        }
        return static_cast<std::size_t>(key);
    }
};

struct TrackedValue {
    inline static int alive = 0;
    inline static int copiesBeforeFailure = -1;
    int id;

    explicit TrackedValue(int identifier) : id(identifier) {
        ++alive;
    }

    TrackedValue(const TrackedValue& other) : id(other.id) {
        if (copiesBeforeFailure == 0) {
            throw std::runtime_error("copy failed");
        }
        if (copiesBeforeFailure > 0) {
            --copiesBeforeFailure;
        }
        ++alive;
    }

    ~TrackedValue() {
        --alive;
    }
};

template <typename Exception, typename Function>
void expectThrow(Function function) {
    bool threw = false;
    try {
        function();
    } catch (const Exception&) {
        threw = true;
    }
    assert(threw);
}

void testBasicOperationsAndCollisions() {
    HashTable<int, int, ConstantHash> table(2);
    assert(table.empty() && table.size() == 0);
    assert(table.find(1) == nullptr && !table.erase(1));
    for (int key = 0; key < 100; ++key) {
        assert(table.insert(key, key * 10));
    }
    assert(table.size() == 100 && table.bucketCount() >= table.size());
    assert(!table.insert(50, 999));
    assert(*table.find(50) == 500);
    *table.find(50) = 501;
    assert(*table.find(50) == 501);

    assert(table.erase(0));
    assert(table.erase(50));
    assert(table.erase(99));
    assert(!table.erase(50));
    assert(table.size() == 97);
    for (int key = 0; key < 100; ++key) {
        const int* value = table.find(key);
        if (key == 0 || key == 50 || key == 99) {
            assert(value == nullptr);
        } else {
            assert(value != nullptr && *value == key * 10);
        }
    }
    for (int key = 0; key < 100; ++key) {
        int* value = table.find(key);
        if (value != nullptr) {
            *value = key + 1;
        }
    }
    const HashTable<int, int, ConstantHash>& readOnly = table;
    int visited = 0;
    for (int key = 0; key < 100; ++key) {
        const int* value = readOnly.find(key);
        if (value != nullptr) {
            assert(*value == key + 1);
            ++visited;
        }
    }
    assert(visited == 97);
    assert(readOnly.find(1000) == nullptr);
    table.rehash(97);
    assert(table.bucketCount() == 97 && *table.find(98) == 99);
    table.clear();
    table.clear();
    assert(table.empty() && table.find(1) == nullptr);
    table.rehash(1);
    assert(table.insert(-5, 42) && *table.find(-5) == 42);
    assert(table.erase(-5) && table.empty());
    assert(table.insert(7, 70) && *table.find(7) == 70);
}

void testStringKeysAndPointerValues() {
    HashTable<std::string, int> names;
    assert(names.insert("Film one", 1));
    assert(names.insert("Film two", 2));
    assert(names.insert("", 3));
    assert(*names.find("Film one") == 1 && *names.find("") == 3);
    assert(names.find("missing") == nullptr);
    assert(!names.insert("Film one", 9));
    assert(names.erase("Film two"));

    assert(TrackedValue::alive == 0);
    {
        TrackedValue entity(8);
        {
            HashTable<int, TrackedValue*> index(1);
            index.insert(1, &entity);
            index.insert(2, &entity);
            assert(*index.find(1) == &entity && *index.find(2) == &entity);
            index.clear();
            assert(TrackedValue::alive == 1);
            index.insert(3, &entity);
        }
        assert(TrackedValue::alive == 1);
    }
    assert(TrackedValue::alive == 0);
}

void testFailures() {
    expectThrow<std::invalid_argument>([] { HashTable<int, int> invalid(0); });
    expectThrow<std::length_error>([] {
        HashTable<int, int> invalid(std::numeric_limits<std::size_t>::max());
    });
    {
        HashTable<int, int> table(2);
        expectThrow<std::invalid_argument>([&] { table.rehash(0); });
        table.insert(1, 10);
        table.insert(2, 20);
        expectThrow<std::invalid_argument>([&] { table.rehash(1); });
        assert(table.size() == 2 && *table.find(1) == 10);
    }
    {
        TrackedValue value(10);
        HashTable<int, TrackedValue> table(1);
        table.insert(1, value);
        const int copyBudgets[] = {0, 1, 2, 3};
        for (int copyBudget : copyBudgets) {
            TrackedValue::copiesBeforeFailure = copyBudget;
            expectThrow<std::runtime_error>([&] { table.insert(2, value); });
            TrackedValue::copiesBeforeFailure = -1;
            assert(table.size() == 1 && table.find(2) == nullptr);
            assert(table.find(1)->id == 10 && TrackedValue::alive == 2);
        }
        table.insert(2, value);
        const std::size_t capacity = table.bucketCount();
        TrackedValue::copiesBeforeFailure = 3;
        expectThrow<std::runtime_error>([&] { table.rehash(16); });
        TrackedValue::copiesBeforeFailure = -1;
        assert(table.size() == 2 && table.bucketCount() == capacity);
        assert(table.find(1)->id == 10 && table.find(2)->id == 10);
        assert(TrackedValue::alive == 3);
    }
    assert(TrackedValue::alive == 0);
    {
        HashTable<int, int, FailingHash> table;
        table.insert(1, 10);
        table.insert(2, 20);
        FailingHash::failingKey = 2;
        expectThrow<std::runtime_error>([&] { table.rehash(16); });
        expectThrow<std::runtime_error>([&] { table.erase(2); });
        expectThrow<std::runtime_error>([&] { table.insert(2, 22); });
        FailingHash::failingKey = -1;
        assert(table.size() == 2 && table.bucketCount() == 8);
        assert(*table.find(1) == 10 && *table.find(2) == 20);
    }
}

void testAliasedInsertAndRepositoryIndex() {
    HashTable<int, std::string> values(1);
    values.insert(1, "existing value");
    const std::string* existing = values.find(1);
    assert(values.insert(2, *existing));
    assert(*values.find(1) == "existing value" && *values.find(2) == "existing value");

    struct SeatSample {
        int id;
        std::size_t row;
        std::size_t column;
    };
    LinkedList<SeatSample> records;
    HashTable<int, SeatSample*> index(1);
    Matrix2D<int> grid(6, 8);
    SeatSample* first = nullptr;
    for (int id = 1; id <= 48; ++id) {
        const std::size_t row = static_cast<std::size_t>(id - 1) / 8;
        const std::size_t column = static_cast<std::size_t>(id - 1) % 8;
        SeatSample& seat = records.pushBack({id, row, column});
        assert(index.insert(id, &seat));
        grid.at(row, column) = id;
        if (id == 1) {
            first = &seat;
        }
    }
    assert(*index.find(1) == first);
    for (int id = 1; id <= 48; ++id) {
        const SeatSample* seat = *index.find(id);
        assert(grid.at(seat->row, seat->column) == id);
    }
    const SeatSample* removed = *index.find(11);
    assert(index.erase(11));
    assert(records.erase(removed));
    grid.at(1, 2) = 0;
    assert(index.find(11) == nullptr && records.size() == 47);
    assert(*index.find(1) == first && first->id == 1);
}

void testRepeatedOperations() {
    HashTable<int, int> table(1);
    int expected[127] = {};
    bool present[127] = {};
    unsigned int state = 12345;
    for (int step = 0; step < 5000; ++step) {
        state = state * 1664525u + 1013904223u;
        const int key = static_cast<int>((state >> 8) % 127);
        switch (state % 3) {
        case 0:
            assert(table.insert(key, step) == !present[key]);
            if (!present[key]) {
                expected[key] = step;
                present[key] = true;
            }
            break;
        case 1:
            assert(table.erase(key) == present[key]);
            present[key] = false;
            break;
        default:
            if (present[key]) {
                *table.find(key) = step;
                expected[key] = step;
            }
        }
        std::size_t count = 0;
        for (int probe = 0; probe < 127; ++probe) {
            const int* value = table.find(probe);
            assert((value != nullptr) == present[probe]);
            if (value != nullptr) {
                assert(*value == expected[probe]);
                ++count;
            }
        }
        assert(table.size() == count);
    }
}

int main() {
    static_assert(!std::is_copy_constructible_v<HashTable<int, int>>);
    static_assert(!std::is_copy_assignable_v<HashTable<int, int>>);
    static_assert(!std::is_move_constructible_v<HashTable<int, int>>);
    static_assert(!std::is_move_assignable_v<HashTable<int, int>>);
    static_assert(std::is_same_v<decltype(std::declval<const HashTable<int, int>&>().find(1)), const int*>);
    testBasicOperationsAndCollisions();
    testStringKeysAndPointerValues();
    testFailures();
    testAliasedInsertAndRepositoryIndex();
    testRepeatedOperations();
    std::cout << "HashTable checks passed\n";
}
