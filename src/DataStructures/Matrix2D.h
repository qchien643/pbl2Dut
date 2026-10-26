#pragma once

#include <cstddef>
#include <limits>
#include <stdexcept>

template <typename T>
class Matrix2D {
public:
    Matrix2D(std::size_t rows, std::size_t columns);

    Matrix2D(const Matrix2D&) = delete;
    Matrix2D& operator=(const Matrix2D&) = delete;
    Matrix2D(Matrix2D&&) = delete;
    Matrix2D& operator=(Matrix2D&&) = delete;

    ~Matrix2D();

    std::size_t rows() const;
    std::size_t columns() const;
    std::size_t size() const;

    T& at(std::size_t row, std::size_t column);
    const T& at(std::size_t row, std::size_t column) const;
    void fill(const T& value);

private:
    void checkBounds(std::size_t row, std::size_t column) const;

    std::size_t rows_;
    std::size_t columns_;
    T* values_ = nullptr;
};

template <typename T>
Matrix2D<T>::Matrix2D(std::size_t rows, std::size_t columns)
    : rows_(rows), columns_(columns)
{
    if (rows == 0 || columns == 0) {
        throw std::invalid_argument("Matrix dimensions must be positive");
    }
    const std::size_t maximum = std::numeric_limits<std::size_t>::max();
    if (rows > maximum / columns || rows * columns > maximum / sizeof(T)) {
        throw std::length_error("Matrix allocation size overflow");
    }
    values_ = new T[rows * columns]{};
}

template <typename T>
Matrix2D<T>::~Matrix2D()
{
    delete[] values_;
}

template <typename T>
std::size_t Matrix2D<T>::rows() const
{
    return rows_;
}

template <typename T>
std::size_t Matrix2D<T>::columns() const
{
    return columns_;
}

template <typename T>
std::size_t Matrix2D<T>::size() const
{
    return rows_ * columns_;
}

template <typename T>
T& Matrix2D<T>::at(std::size_t row, std::size_t column)
{
    checkBounds(row, column);
    return values_[row * columns_ + column];
}

template <typename T>
const T& Matrix2D<T>::at(std::size_t row, std::size_t column) const
{
    checkBounds(row, column);
    return values_[row * columns_ + column];
}

template <typename T>
void Matrix2D<T>::fill(const T& value)
{
    for (std::size_t index = 0; index < size(); ++index) {
        values_[index] = value;
    }
}

template <typename T>
void Matrix2D<T>::checkBounds(std::size_t row, std::size_t column) const
{
    if (row >= rows_ || column >= columns_) {
        throw std::out_of_range("Matrix index out of range");
    }
}
