#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <ratio>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "neh.hpp"

#define BENCHMARK

namespace {
using namespace std::literals;

constexpr auto *DATA_PATH = "data/";
constexpr auto *INSTANCES_FILENAME = "instances.txt";

auto split(const std::string &str, char delim) {
  auto sstream = std::stringstream{str};
  auto out = std::vector<std::string>{};
  auto token = std::string{};

  while (std::getline(sstream, token, delim)) {
    out.push_back(std::move(token));
  }
  return out;
}

auto get_lines(const std::string &filepath) {
  auto istream = std::ifstream{filepath};
  auto out = std::vector<std::string>{};
  auto line = std::string{};

  while (std::getline(istream, line)) {
    out.push_back(std::move(line));
  }
  return out;
}

template <typename InstanceDataType>
auto read_instance_data(const std::string &filepath) {
  const auto lines = get_lines(filepath);
  const auto dimensions = split(lines[1], ' ');
  const auto number_jobs = std::stoull(dimensions[0]);
  const auto number_machines = std::stoull(dimensions[1]);

  auto processing_times = std::vector<InstanceDataType>{};
  auto jobs = std::vector<neh::Job<InstanceDataType>>{};
  jobs.reserve(number_jobs);

  for (size_t i = 0; i < number_jobs; ++i) {
    processing_times.clear();
    auto time_accum = InstanceDataType{0};

    for (const auto &time_str : split(lines[i + 3], '\t')) {
      const auto time = std::stoull(time_str);
      time_accum += time;
      processing_times.push_back(time);
    }
    jobs.emplace_back(std::move(processing_times), time_accum, i - 1);
  }
  return std::tuple{jobs, number_jobs, number_machines};
}
} // namespace

int main() {
  const auto directory = std::string{DATA_PATH};

  for (const auto &instance : get_lines(directory + INSTANCES_FILENAME)) {
    const auto runs = size_t{200};
    auto min_time =
        std::chrono::microseconds{std::numeric_limits<int64_t>::max()};
    auto max_time = std::chrono::microseconds{0};
    auto time_accum = std::chrono::microseconds{0};

    for (size_t i = 0; i != runs; ++i) {
      auto [jobs, number_jobs, number_machines] =
          read_instance_data<uint32_t>(directory + "/" + instance + ".txt");

      const auto [solution, elapsed] =
          neh::solve(std::move(jobs), number_jobs, number_machines);

      max_time = std::max(max_time, elapsed);
      min_time = std::min(min_time, elapsed);
      time_accum += elapsed;

      if (i == runs - 1) {
        std::cout << "Number of Jobs: " << solution.number_jobs
                  << "\nNumber of Machines: " << solution.number_machines;
        std::cout << "\nNEH makespan: " << neh::calculate_makespan(solution)
                  << "\nNEH makespan with Taillard's acceleration: "
                  << solution.makespan << "\n";
      }
    }
    std::cout << "elapsed avg: " << time_accum.count() / runs << "us\n"
              << "elapsed min: " << min_time.count() << "us\n"
              << "elapsed max: " << max_time.count() << "us\n";
  }
  return 0;
}