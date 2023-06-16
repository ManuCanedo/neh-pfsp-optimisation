#ifndef TAILLARD_HPP
#define TAILLARD_HPP

#include <cassert>
#include <vector>

#include <algorithm>
#include <chrono>
#include <limits>

#include <matrix.h>

namespace neh {
template <typename NumericType>
struct Job {
    Job(std::vector<NumericType> &&processing_times,
        NumericType total_processing_time)
            : processing_times{std::move(processing_times)},
              total_processing_time{total_processing_time} {}

    std::vector<NumericType> processing_times;
    NumericType total_processing_time;
};

template <typename NumericType>
struct Solution {
    Solution(size_t number_jobs, size_t number_machines)
            : number_jobs{number_jobs}, number_machines{number_machines} {
        assert(number_jobs > 1);
        assert(number_machines > 1);
        jobs.reserve(number_jobs);
    }

    std::vector<Job<NumericType>> jobs;
    size_t number_jobs;
    size_t number_machines;
    NumericType makespan{0};
};

template <typename NumericType>
constexpr NumericType max(NumericType lhs, NumericType rhs) {
    return lhs > rhs ? lhs : rhs;
}

template <typename NumericType>
constexpr void populate_e_mat(const std::vector<Job<NumericType>> &jobs,
                              size_t index, Matrix<NumericType> &e_mat) {
    // Calculate element (0, 0)
    e_mat(0, 0) = jobs[0].processing_times[0];

    // Calculate elements (0, j != 0)
    for (auto j = std::size_t{1}; j != e_mat.width(); ++j) {
        e_mat(0, j) = jobs[0].processing_times[j] + e_mat(0, j - 1);
    }
    for (auto i = std::size_t{1}; i != index + 1; ++i) {
        // Calculate element (i != 0, 0)
        e_mat(i, 0) = jobs[i].processing_times[0] + e_mat(i - 1, 0);

        // Calculate element (i != 0, j != 0)
        for (auto j = std::size_t{1}; j != e_mat.width(); ++j) {
            e_mat(i, j) = jobs[i].processing_times[j]
                    + max(e_mat(i - 1, j), e_mat(i, j - 1));
        }
    }
}

template <typename NumericType>
constexpr void populate_q_mat(const std::vector<Job<NumericType>> &jobs,
                              size_t index, Matrix<NumericType> &q_mat) {
    // Set row (index, j) to 0
    std::fill_n(&q_mat(index, 0), q_mat.width(), NumericType{0});
    if (index == 0) {
        return;
    }
    // Calculate element (index - 1, width - 1)
    q_mat(index - 1, q_mat.width() - 1) =
            jobs[index - 1].processing_times[q_mat.width() - 1];

    // Calculate elements (index - 1, j != width - 1)
    for (auto j = q_mat.width() - 1; j-- != 0;) {
        q_mat(index - 1, j) =
                jobs[index - 1].processing_times[j] + q_mat(index - 1, j + 1);
    }
    if (index == 1) {
        return;
    }
    // Calculate elements (i != index - 1, j)
    for (auto i = index - 1; i-- != 0;) {
        // Calculate element (i != index - 1, width - 1)
        q_mat(i, q_mat.width() - 1) =
                jobs[i].processing_times[q_mat.width() - 1]
                + q_mat(i + 1, q_mat.width() - 1);

        // Calculate elements (i != index - 1, j != width - 1)
        for (auto j = q_mat.width() - 1; j-- != 0;) {
            q_mat(i, j) = jobs[i].processing_times[j]
                    + max(q_mat(i + 1, j), q_mat(i, j + 1));
        }
    }
}

template <typename NumericType>
constexpr void populate_f_mat(const std::vector<Job<NumericType>> &jobs,
                              size_t index, const Matrix<NumericType> &e_mat,
                              Matrix<NumericType> &f_mat) {
    // Calculate element (0, 0)
    f_mat(0, 0) = jobs[index].processing_times[0];

    // Calculate elements (0, j != 0)
    for (auto j = std::size_t{1}; j != f_mat.width(); ++j) {
        f_mat(0, j) = jobs[index].processing_times[j] + f_mat(0, j - 1);
    }
    for (auto i = std::size_t{1}; i != index + 1; ++i) {
        // Calculate element (i != 0, 0)
        f_mat(i, 0) = jobs[index].processing_times[0] + e_mat(i - 1, 0);

        // Calculate element (i != 0, j != 0)
        for (auto j = std::size_t{1}; j != f_mat.width(); ++j) {
            f_mat(i, j) = jobs[index].processing_times[j]
                    + max(e_mat(i - 1, j), f_mat(i, j - 1));
        }
    }
}
} // namespace neh

#endif // TAILLARD_HPP