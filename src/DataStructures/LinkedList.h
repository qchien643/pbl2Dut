#pragma once

#include <cstddef>
#include <utility>

template <typename T>
class LinkedList {
private:
    struct Node {
        T value;
        Node* previous;
        Node* next;
    };

public:
    class Iterator {
    public:
        explicit Iterator(Node* node) : current_(node) {}
        T& operator*() const { return current_->value; }
        Iterator& operator++() {
            current_ = current_->next;
            return *this;
        }
        bool operator!=(const Iterator& other) const { return current_ != other.current_; }
    private:
        Node* current_;
    };

    // Read-only traversal; callers cannot change nodes or stored values.
    class ConstIterator {
    public:
        explicit ConstIterator(const Node* node) : current_(node) {}

        const T& operator*() const { return current_->value; }

        ConstIterator& operator++() {
            current_ = current_->next;
            return *this;
        }

        bool operator!=(const ConstIterator& other) const {
            return current_ != other.current_;
        }

    private:
        const Node* current_;
    };

    Iterator begin();
    Iterator end();
    ConstIterator begin() const;
    ConstIterator end() const;

    LinkedList() = default;
    LinkedList(const LinkedList&) = delete;
    LinkedList& operator=(const LinkedList&) = delete;
    LinkedList(LinkedList&&) = delete;
    LinkedList& operator=(LinkedList&&) = delete;

    ~LinkedList();

    void swap(LinkedList& other) noexcept;
    bool empty() const;
    std::size_t size() const;

    T& pushFront(const T& value);
    T& pushBack(const T& value);
    T* find(const T& value);
    const T* find(const T& value) const;

    // Remove the exact stored object, not another object with the same value.
    bool erase(const T* value);

    void clear() noexcept;

private:
    void eraseNode(Node* node) noexcept;

    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    std::size_t size_ = 0;
};

template <typename T>
typename LinkedList<T>::Iterator LinkedList<T>::begin()
{
    return Iterator(head_);
}

template <typename T>
typename LinkedList<T>::Iterator LinkedList<T>::end()
{
    return Iterator(nullptr);
}

template <typename T>
typename LinkedList<T>::ConstIterator LinkedList<T>::begin() const
{
    return ConstIterator(head_);
}

template <typename T>
typename LinkedList<T>::ConstIterator LinkedList<T>::end() const
{
    return ConstIterator(nullptr);
}

template <typename T>
LinkedList<T>::~LinkedList()
{
    clear();
}

template <typename T>
void LinkedList<T>::swap(LinkedList& other) noexcept
{
    std::swap(head_, other.head_);
    std::swap(tail_, other.tail_);
    std::swap(size_, other.size_);
}

template <typename T>
bool LinkedList<T>::empty() const
{
    return size_ == 0;
}

template <typename T>
std::size_t LinkedList<T>::size() const
{
    return size_;
}

template <typename T>
T& LinkedList<T>::pushFront(const T& value)
{
    Node* node = new Node{value, nullptr, head_};
    if (head_ != nullptr) {
        head_->previous = node;
    } else {
        tail_ = node;
    }
    head_ = node;
    ++size_;
    return node->value;
}

template <typename T>
T& LinkedList<T>::pushBack(const T& value)
{
    Node* node = new Node{value, tail_, nullptr};
    if (tail_ != nullptr) {
        tail_->next = node;
    } else {
        head_ = node;
    }
    tail_ = node;
    ++size_;
    return node->value;
}

template <typename T>
T* LinkedList<T>::find(const T& value)
{
    for (Node* current = head_; current != nullptr; current = current->next) {
        if (current->value == value) {
            return &current->value;
        }
    }
    return nullptr;
}

template <typename T>
const T* LinkedList<T>::find(const T& value) const
{
    for (const Node* current = head_; current != nullptr; current = current->next) {
        if (current->value == value) {
            return &current->value;
        }
    }
    return nullptr;
}

template <typename T>
bool LinkedList<T>::erase(const T* value)
{
    for (Node* current = head_; current != nullptr; current = current->next) {
        if (&current->value == value) {
            eraseNode(current);
            return true;
        }
    }
    return false;
}

template <typename T>
void LinkedList<T>::clear() noexcept
{
    while (head_ != nullptr) {
        Node* next = head_->next;
        delete head_;
        head_ = next;
    }
    tail_ = nullptr;
    size_ = 0;
}

template <typename T>
void LinkedList<T>::eraseNode(Node* node) noexcept
{
    if (node->previous != nullptr) {
        node->previous->next = node->next;
    } else {
        head_ = node->next;
    }
    if (node->next != nullptr) {
        node->next->previous = node->previous;
    } else {
        tail_ = node->previous;
    }
    delete node;
    --size_;
}
