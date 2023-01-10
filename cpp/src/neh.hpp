#ifndef NEH_HPP
#define NEH_HPP

#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>

namespace neh
{
template <typename NumericType>
struct Job {
    Job(std::vector<NumericType>&& processing_times, NumericType total_processing_time, size_t id)
        : processing_times{std::move(processing_times)},
          id{id},
          total_processing_time{total_processing_time}
    {
    }

    std::vector<NumericType> processing_times;
    size_t id;
    NumericType total_processing_time;
};

template<typename NumericType>
struct Solution {
    Solution(size_t number_jobs, size_t number_machines)
        : number_jobs{number_jobs},
        number_machines{number_machines},
        makespan{0}
    {
        jobs.reserve(number_jobs);
    }
    ~Solution() = default;
    Solution(const Solution&) = delete;
    Solution& operator=(const Solution&) = delete;
    Solution(Solution&&) noexcept = default;
    Solution& operator=(Solution&&) = default;

    std::vector<Job<NumericType>> jobs;
    size_t number_jobs;
    size_t number_machines;
    NumericType makespan;
};

template<typename NumericType>
class Matrix {
public:
    Matrix(size_t rows, size_t cols) : data_(rows * cols), cols_(cols) {}

    NumericType& at(size_t i, size_t j) {
        return data_[i * cols_ + j];
    }
    const NumericType& at(size_t i, size_t j) const {
        return data_[i * cols_ + j];
    }

private:
    std::vector<NumericType> data_;
    size_t cols_;
};

template <typename NumericType>
auto calculate_e_mat(const Solution<NumericType> &solution, const size_t index, Matrix<NumericType>& e_mat)
{
    const NumericType* pt_i = solution.jobs[0].processing_times.data();

    // Calculate element (0, 0)
    e_mat.at(0, 0) = pt_i[0];

    // Calculate elements (0, j != 0)
    for (size_t j = 1; j < solution.number_machines; ++j) {
        e_mat.at(0, j) = pt_i[j] + e_mat.at(0, j-1);
    }
    for (size_t i = 1; i <= index; ++i) {
        pt_i = solution.jobs[i].processing_times.data();

        // Calculate element (i != 0, 0)
        e_mat.at(i, 0) = pt_i[0] + e_mat.at(i-1, 0);

        // Calculate element (i != 0, j != 0)
        for (size_t j = 1; j < solution.number_machines; ++j) {
            e_mat.at(i, j) =  pt_i[j] + std::max(e_mat.at(i-1, j), e_mat.at(i, j-1));
        }
    }
}

template <typename NumericType>
auto calculate_q_mat(const Solution<NumericType> &solution, const size_t index, Matrix<NumericType>& q_mat)
{
    const NumericType* pt_i = nullptr;

    // Set row (index, j) to 0
    for (size_t j = 0; j < solution.number_machines; ++j) {
        q_mat.at(index, j) = 0;
    }
    // Calculate elements (i != index, j)
    for (ssize_t i = index - 1; i >= 0; --i) {
        pt_i = solution.jobs[i].processing_times.data();

        for (ssize_t j = solution.number_machines - 1; j >= 0; --j) {
            q_mat.at(i, j) = pt_i[j] + std::max(
                    i == index - 1 ? 0 : q_mat.at(i+1, j),
                    j == solution.number_machines - 1 ? 0 : q_mat.at(i, j+1));
        }
    }
}

template <typename NumericType>
void calculate_f_mat(const Solution<NumericType> &solution, const size_t index, const Matrix<NumericType>& e_mat, Matrix<NumericType>& f_mat)
{
    const NumericType* pt_index = solution.jobs[index].processing_times.data();

    // Calculate element (0, 0)
    f_mat.at(0, 0) = pt_index[0];

    // Calculate elements (0, j != 0)
    for (size_t j = 1; j < solution.number_machines; ++j) {
        f_mat.at(0, j) = pt_index[j] + f_mat.at(0, j-1);
    }
    for (size_t i = 1; i <= index; ++i) {
        // Calculate element (i != 0, 0)
        f_mat.at(i, 0) =  pt_index[0] + e_mat.at(i-1, 0);

        // Calculate element (i != 0, j != 0)
        for (size_t j = 1; j < solution.number_machines; ++j) {
            f_mat.at(i, j) =  pt_index[j] + std::max(e_mat.at(i-1, j), f_mat.at(i, j-1));
        }
    }
}

template <typename NumericType>
auto try_shift_improve(Solution<NumericType> &solution, const size_t index, Matrix<NumericType>& eq_mat, Matrix<NumericType>& f_mat)
{
    calculate_e_mat(solution, index, eq_mat);
    calculate_f_mat(solution, index, eq_mat, f_mat);
    calculate_q_mat(solution, index, eq_mat);
    auto best_index = index;
    auto best_makespan = std::numeric_limits<NumericType>::max();

    for (ssize_t i = 0; i <= index; ++i) {
        auto max_sum = 0;
        for (size_t j = 0; j < solution.number_machines; ++j) {
            const auto sum = f_mat.at(i, j) + eq_mat.at(i, j);
            if (sum > max_sum) {
                max_sum = sum;
            }
        }
        if (max_sum < best_makespan) {
            best_index = i;
            best_makespan = max_sum;
        }
    }
    if (best_index < index) {
        auto tmp = std::move(solution.jobs[index]);
        for (size_t j = index; j >= best_index + 1; --j) {
            solution.jobs[j] = std::move(solution.jobs[j-1]);
        }
        solution.jobs[best_index] = std::move(tmp);
    }
    if (index == solution.number_jobs - 1) {
        solution.makespan = best_makespan;
    }
    return best_index;
}

template <typename NumericType>
auto solve(std::vector<Job<NumericType>>&& jobs, const size_t number_jobs, const size_t number_machines)
{
    auto eq_mat = Matrix<NumericType>(number_jobs, number_machines);
    auto f_mat = Matrix<NumericType>(number_jobs, number_machines);
    auto solution = Solution<NumericType>{number_jobs, number_machines};

    std::sort(jobs.begin(), jobs.end(), [](auto&& job1, auto&& job2) {
        return job1.total_processing_time > job2.total_processing_time;
    });
    for (size_t i = 0; i < jobs.size(); ++i) {
        solution.jobs.emplace_back(std::move(jobs[i]));
        try_shift_improve(solution, i, eq_mat, f_mat);
    }
    return solution;
}

template <typename NumericType>
auto calculate_makespan(const Solution<NumericType> &solution)
{
    auto times =
        std::vector<NumericType>(solution.number_jobs * solution.number_machines, 0);
    for (size_t i = 0; i < solution.number_jobs; ++i) {
        for (size_t j = 0; j < solution.number_machines; ++j) {
            const auto i_factor =
                i == 0 ?  0 : times[(i - 1) * solution.number_machines + j];
            const auto j_factor =
                j == 0 ?  0 : times[i * solution.number_machines + j - 1];
            times[i * solution.number_machines + j] =
                solution.jobs[i].processing_times[j] +
                std::max(i_factor, j_factor);
        }
    }
    return times.back();
}
} // namespace neh

#endif // NEH_HPP