#pragma once

#include <cstddef>
#include <utility>

namespace cinema {

template <typename T>
class DoubleLinkedList {
private:
    struct Node {
        explicit Node(const T& item) : value(item), previous(nullptr), next(nullptr) {}
        explicit Node(T&& item) : value(std::move(item)), previous(nullptr), next(nullptr) {}

        T value;
        Node* previous;
        Node* next;
    };

public:
    DoubleLinkedList() = default;

    DoubleLinkedList(const DoubleLinkedList& other) {
        try {
            other.forEach([this](const T& value) { append(value); });
        } catch (...) {
            clear();
            throw;
        }
    }

    DoubleLinkedList& operator=(const DoubleLinkedList& other) {
        if (this != &other) {
            DoubleLinkedList copy(other);
            swap(copy);
        }
        return *this;
    }

    DoubleLinkedList(DoubleLinkedList&& other) noexcept
        : head_(other.head_), tail_(other.tail_), size_(other.size_) {
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }

    DoubleLinkedList& operator=(DoubleLinkedList&& other) noexcept {
        if (this != &other) {
            clear();
            head_ = other.head_;
            tail_ = other.tail_;
            size_ = other.size_;
            other.head_ = nullptr;
            other.tail_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    ~DoubleLinkedList() {
        clear();
    }

    T* append(const T& value) {
        return appendNode(new Node(value));
    }

    T* append(T&& value) {
        return appendNode(new Node(std::move(value)));
    }

    template <typename Compare>
    T* insertSorted(const T& value, Compare compare) {
        Node* current = head_;
        while (current && !compare(value, current->value)) {
            current = current->next;
        }

        Node* inserted = new Node(value);
        if (!current) {
            appendNode(inserted);
            return &inserted->value;
        }
        if (current->previous) {
            current->previous->next = inserted;
        } else {
            head_ = inserted;
        }
        inserted->previous = current->previous;
        inserted->next = current;
        current->previous = inserted;
        ++size_;
        return &inserted->value;
    }

    template <typename Predicate>
    bool eraseFirst(Predicate predicate) {
        Node* current = head_;
        while (current) {
            if (predicate(current->value)) {
                unlink(current);
                return true;
            }
            current = current->next;
        }
        return false;
    }

    template <typename Predicate>
    std::size_t eraseIf(Predicate predicate) {
        std::size_t erasedCount = 0;
        Node* current = head_;
        while (current) {
            if (predicate(current->value)) {
                Node* removed = current;
                current = current->next;
                unlink(removed);
                ++erasedCount;
            } else {
                current = current->next;
            }
        }
        return erasedCount;
    }

    template <typename Predicate>
    T* findIf(Predicate predicate) {
        for (Node* node = head_; node; node = node->next) {
            if (predicate(node->value)) {
                return &node->value;
            }
        }
        return nullptr;
    }

    template <typename Predicate>
    const T* findIf(Predicate predicate) const {
        for (const Node* node = head_; node; node = node->next) {
            if (predicate(node->value)) {
                return &node->value;
            }
        }
        return nullptr;
    }

    template <typename Function>
    void forEach(Function function) {
        for (Node* node = head_; node; node = node->next) {
            function(node->value);
        }
    }

    template <typename Function>
    void forEach(Function function) const {
        for (const Node* node = head_; node; node = node->next) {
            function(node->value);
        }
    }

    void clear() noexcept {
        while (head_) {
            Node* next = head_->next;
            delete head_;
            head_ = next;
        }
        tail_ = nullptr;
        size_ = 0;
    }

    std::size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }

    void swap(DoubleLinkedList& other) noexcept {
        Node* savedHead = head_;
        head_ = other.head_;
        other.head_ = savedHead;

        Node* savedTail = tail_;
        tail_ = other.tail_;
        other.tail_ = savedTail;

        const std::size_t savedSize = size_;
        size_ = other.size_;
        other.size_ = savedSize;
    }

private:
    T* appendNode(Node* node) {
        if (tail_) {
            node->previous = tail_;
            tail_->next = node;
        } else {
            head_ = node;
        }
        tail_ = node;
        ++size_;
        return &node->value;
    }

    void unlink(Node* removed) noexcept {
        if (removed->previous) {
            removed->previous->next = removed->next;
        } else {
            head_ = removed->next;
        }
        if (removed->next) {
            removed->next->previous = removed->previous;
        }
        if (tail_ == removed) {
            tail_ = removed->previous;
        }
        delete removed;
        --size_;
    }

    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    std::size_t size_ = 0;
};

} // namespace cinema
