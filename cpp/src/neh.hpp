#ifndef NEH_HPP
#define NEH_HPP

#include <algorithm>
#include <cassert>
#include <chrono>
#include <limits>
#include <vector>

namespace neh {
template <typename NumericType> struct Job {
  Job(std::vector<NumericType> &&processing_times,
      NumericType total_processing_time, size_t id)
      : processing_times{std::move(processing_times)}, id{id},
        total_processing_time{total_processing_time} {}

  std::vector<NumericType> processing_times;
  size_t id;
  NumericType total_processing_time;
};

template <typename NumericType> struct Solution {
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

template <typename NumericType> class Matrix {
public:
  Matrix(size_t rows, size_t cols)
      : data_(rows, std::vector<NumericType>(cols, 0)), cols_(cols) {}

  NumericType &operator()(size_t i, size_t j) { return data_[i][j]; }

  const NumericType &operator()(size_t i, size_t j) const {
    return data_[i][j];
  }

  size_t width() const { return cols_; }
  NumericType back() const { return data_.back().back(); }

private:
  std::vector<std::vector<NumericType>> data_;
  size_t cols_;
};

template <typename NumericType>
auto populate_e_mat(const std::vector<Job<NumericType>> &jobs, size_t index,
                    Matrix<NumericType> &e_mat) {
  // Calculate element (0, 0)
  e_mat(0, 0) = jobs[0].processing_times[0];

  // Calculate elements (0, j != 0)
  for (size_t j = 1; j < e_mat.width(); ++j) {
    e_mat(0, j) = jobs[0].processing_times[j] + e_mat(0, j - 1);
  }
  for (size_t i = 1; i <= index; ++i) {
    // Calculate element (i != 0, 0)
    e_mat(i, 0) = jobs[i].processing_times[0] + e_mat(i - 1, 0);

    // Calculate element (i != 0, j != 0)
    for (size_t j = 1; j < e_mat.width(); ++j) {
      e_mat(i, j) = jobs[i].processing_times[j] +
                    std::max(e_mat(i - 1, j), e_mat(i, j - 1));
    }
  }
}

template <typename NumericType>
auto populate_q_mat(const std::vector<Job<NumericType>> &jobs, size_t index,
                    Matrix<NumericType> &q_mat) {
  // Set row (index, j) to 0
  for (size_t j = 0; j < q_mat.width(); ++j) {
    q_mat(index, j) = 0;
  }
  if (index == 0) {
    return;
  }
  q_mat(index - 1, q_mat.width() - 1) =
      jobs[index - 1].processing_times[q_mat.width() - 1];

  for (ssize_t j = q_mat.width() - 2; j >= 0; --j) {
    q_mat(index - 1, j) =
        jobs[index - 1].processing_times[j] + q_mat(index - 1, j + 1);
  }
  if (index == 1) {
    return;
  }
  // Calculate elements (i != index, j)
  for (ssize_t i = index - 2; i >= 0; --i) {
    // Calculate element (i != 0, 0)
    q_mat(i, q_mat.width() - 1) = jobs[i].processing_times[q_mat.width() - 1] +
                                  q_mat(i + 1, q_mat.width() - 1);

    for (ssize_t j = q_mat.width() - 2; j >= 0; --j) {
      q_mat(i, j) = jobs[i].processing_times[j] +
                    std::max(q_mat(i + 1, j), q_mat(i, j + 1));
    }
  }
}

template <typename NumericType>
void populate_f_mat(const std::vector<Job<NumericType>> &jobs, size_t index,
                    const Matrix<NumericType> &e_mat,
                    Matrix<NumericType> &f_mat) {
  // Calculate element (0, 0)
  f_mat(0, 0) = jobs[index].processing_times[0];

  // Calculate elements (0, j != 0)
  for (size_t j = 1; j < f_mat.width(); ++j) {
    f_mat(0, j) = jobs[index].processing_times[j] + f_mat(0, j - 1);
  }
  for (size_t i = 1; i <= index; ++i) {
    // Calculate element (i != 0, 0)
    f_mat(i, 0) = jobs[index].processing_times[0] + e_mat(i - 1, 0);
    
    // Calculate element (i != 0, j != 0)
    for (size_t j = 1; j < f_mat.width(); ++j) {
      f_mat(i, j) = jobs[index].processing_times[j] +
                    std::max(e_mat(i - 1, j), f_mat(i, j - 1));
    }
  }
}

template <typename NumericType>
auto try_shift_improve(Solution<NumericType> &solution, size_t index,
                       Matrix<NumericType> &eq_mat,
                       Matrix<NumericType> &f_mat) {
  populate_e_mat(solution.jobs, index, eq_mat);
  populate_f_mat(solution.jobs, index, eq_mat, f_mat);
  populate_q_mat(solution.jobs, index, eq_mat);
  auto best_index = index;
  auto best_makespan = std::numeric_limits<NumericType>::max();

  for (ssize_t i = 0; i <= index; ++i) {
    auto max_sum = NumericType{0};

    for (size_t j = 0; j < f_mat.width(); ++j) {
      max_sum = std::max(f_mat(i, j) + eq_mat(i, j), max_sum);
    }
    if (max_sum < best_makespan) {
      best_index = i;
      best_makespan = max_sum;
    }
  }
  if (best_index < index) {
    auto tmp = std::move(solution.jobs[index]);

    for (size_t j = index; j >= best_index + 1; --j) {
      solution.jobs[j] = std::move(solution.jobs[j - 1]);
    }
    solution.jobs[best_index] = std::move(tmp);
  }
  if (index == solution.number_jobs - 1) {
    solution.makespan = best_makespan;
  }
  return best_index;
}

template <typename NumericType>
auto solve(std::vector<Job<NumericType>> &&jobs, size_t number_jobs,
           const size_t number_machines) {
  auto eq_mat = Matrix<NumericType>(number_jobs, number_machines);
  auto f_mat = Matrix<NumericType>(number_jobs, number_machines);
  auto solution = Solution<NumericType>{number_jobs, number_machines};

  const auto start = std::chrono::high_resolution_clock::now();
  
  std::sort(jobs.begin(), jobs.end(), [](auto &&job1, auto &&job2) {
    return job1.total_processing_time > job2.total_processing_time;
  });
  solution.jobs.emplace_back(std::move(jobs[0]));

  for (size_t i = 1; i < jobs.size(); ++i) {
    solution.jobs.emplace_back(std::move(jobs[i]));
    try_shift_improve(solution, i, eq_mat, f_mat);
  }

  const auto end = std::chrono::high_resolution_clock::now();
  return std::tuple{
      std::move(solution),
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)};
}

template <typename NumericType>
auto calculate_makespan(const Solution<NumericType> &solution) {
  auto t_mat =
      Matrix<NumericType>{solution.number_jobs, solution.number_machines};

  for (size_t i = 0; i < solution.number_jobs; ++i) {
    for (size_t j = 0; j < t_mat.width(); ++j) {
      t_mat(i, j) =
          solution.jobs[i].processing_times[j] +
          std::max(i == 0 ? 0 : t_mat(i - 1, j), j == 0 ? 0 : t_mat(i, j - 1));
    }
  }
  return t_mat.back();
}
} // namespace neh

#endif // NEH_HPP
