#ifndef NEH_HPP
#define NEH_HPP

#include <algorithm>
#include <chrono>
#include <limits>
#include <vector>

#include <neh.h>

namespace neh {
template <typename NumericType>
constexpr auto try_shift_improve(Solution<NumericType> &solution,
                                 std::size_t index, Matrix<NumericType> &eq_mat,
                                 Matrix<NumericType> &f_mat) {
    populate_e_mat(solution.jobs, index, eq_mat);
    populate_f_mat(solution.jobs, index, eq_mat, f_mat);
    populate_q_mat(solution.jobs, index, eq_mat);

    auto best_index = index;
    solution.makespan = std::numeric_limits<NumericType>::max();

    for (auto i = std::size_t{0}; i != index + 1; ++i) {
        auto max_sum = NumericType{0};

        for (auto j = std::size_t{0}; j != f_mat.width(); ++j) {
            max_sum = max(f_mat(i, j) + eq_mat(i, j), max_sum);
        }
        if (max_sum < solution.makespan) {
            best_index = i;
            solution.makespan = max_sum;
        }
    }
    if (best_index != index) {
        std::rotate(solution.jobs.begin() + best_index,
                    solution.jobs.begin() + index,
                    solution.jobs.begin() + index + 1);
    }
    return best_index;
}

template <typename NumericType>
constexpr auto solve(std::vector<Job<NumericType>> &&jobs,
                     std::size_t number_jobs, std::size_t number_machines) {
    auto eq_mat = Matrix<NumericType>{number_jobs, number_machines};
    auto f_mat = Matrix<NumericType>{number_jobs, number_machines};
    auto solution = Solution<NumericType>{number_jobs, number_machines};

    const auto start = std::chrono::high_resolution_clock::now();

    std::sort(jobs.begin(), jobs.end(), [](auto &&job1, auto &&job2) {
        return job1.total_processing_time > job2.total_processing_time;
    });
    solution.jobs.emplace_back(std::move(jobs[0]));

    for (auto i = std::size_t{1}; i != jobs.size(); ++i) {
        solution.jobs.emplace_back(std::move(jobs[i]));
        try_shift_improve(solution, i, eq_mat, f_mat);
    }

    const auto end = std::chrono::high_resolution_clock::now();
    return std::tuple{
            std::move(solution),
            std::chrono::duration_cast<std::chrono::microseconds>(end - start)};
}

template <typename NumericType>
constexpr auto calculate_makespan(const Solution<NumericType> &solution) {
    auto t_mat =
            Matrix<NumericType>{solution.number_jobs, solution.number_machines};

    for (auto i = std::size_t{0}; i != t_mat.height(); ++i) {
        for (auto j = std::size_t{0}; j != t_mat.width(); ++j) {
            t_mat(i, j) = solution.jobs[i].processing_times[j]
                    + max(i == 0 ? 0 : t_mat(i - 1, j),
                          j == 0 ? 0 : t_mat(i, j - 1));
        }
    }
    return t_mat(t_mat.height() - 1, t_mat.width() - 1);
}
} // namespace neh

#endif // NEH_HPP
