#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <cstddef>
#include <vector>

namespace neh {
template <typename NumericType>
class Matrix {
public:
    Matrix(std::size_t rows, std::size_t cols)
            : data_(rows, std::vector<NumericType>(cols)),
              cols_(cols),
              rows_(rows) {}

    NumericType &operator()(std::size_t i, std::size_t j) {
        return const_cast<NumericType &>(
                const_cast<const Matrix &>(*this)(i, j));
    }

    const NumericType &operator()(std::size_t i, std::size_t j) const {
        return data_[i][j];
    }

    std::size_t width() const { return cols_; }
    std::size_t height() const { return rows_; }

private:
    std::vector<std::vector<NumericType>> data_;
    std::size_t cols_;
    std::size_t rows_;
};
} // namespace neh

#endif // MATRIX_HPP