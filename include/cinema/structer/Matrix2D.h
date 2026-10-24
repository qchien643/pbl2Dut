#pragma once

#include <cstddef>
#include <limits>
#include <stdexcept>

namespace cinema {

template <typename T>
class Matrix2D {
public:
    Matrix2D() = default;

    Matrix2D(std::size_t rows, std::size_t columns, const T& initial = T{})
        : rows_(rows), columns_(columns) {
        if (columns_ != 0 && rows_ > std::numeric_limits<std::size_t>::max() / columns_) {
            throw std::length_error("Matrix2D dimensions overflow");
        }
        const std::size_t count = rows_ * columns_;
        if (count == 0) {
            return;
        }

        data_ = new T[count];
        try {
            for (std::size_t index = 0; index < count; ++index) {
                data_[index] = initial;
            }
        } catch (...) {
            delete[] data_;
            data_ = nullptr;
            throw;
        }
    }

    Matrix2D(const Matrix2D& other)
        : rows_(other.rows_), columns_(other.columns_) {
        const std::size_t count = rows_ * columns_;
        if (count == 0) {
            return;
        }

        data_ = new T[count];
        try {
            for (std::size_t index = 0; index < count; ++index) {
                data_[index] = other.data_[index];
            }
        } catch (...) {
            delete[] data_;
            data_ = nullptr;
            throw;
        }
    }

    Matrix2D& operator=(const Matrix2D& other) {
        if (this != &other) {
            Matrix2D copy(other);
            swap(copy);
        }
        return *this;
    }

    Matrix2D(Matrix2D&& other) noexcept
        : rows_(other.rows_), columns_(other.columns_), data_(other.data_) {
        other.rows_ = 0;
        other.columns_ = 0;
        other.data_ = nullptr;
    }

    Matrix2D& operator=(Matrix2D&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            rows_ = other.rows_;
            columns_ = other.columns_;
            data_ = other.data_;
            other.rows_ = 0;
            other.columns_ = 0;
            other.data_ = nullptr;
        }
        return *this;
    }

    ~Matrix2D() {
        delete[] data_;
    }

    T& at(std::size_t row, std::size_t column) {
        checkBounds(row, column);
        return data_[row * columns_ + column];
    }

    const T& at(std::size_t row, std::size_t column) const {
        checkBounds(row, column);
        return data_[row * columns_ + column];
    }

    std::size_t rows() const noexcept { return rows_; }
    std::size_t columns() const noexcept { return columns_; }

    void swap(Matrix2D& other) noexcept {
        const std::size_t savedRows = rows_;
        rows_ = other.rows_;
        other.rows_ = savedRows;

        const std::size_t savedColumns = columns_;
        columns_ = other.columns_;
        other.columns_ = savedColumns;

        T* savedData = data_;
        data_ = other.data_;
        other.data_ = savedData;
    }

private:
    void checkBounds(std::size_t row, std::size_t column) const {
        if (row >= rows_ || column >= columns_) {
            throw std::out_of_range("Matrix2D index out of range");
        }
    }

    std::size_t rows_ = 0;
    std::size_t columns_ = 0;
    T* data_ = nullptr;
};

} // namespace cinema
