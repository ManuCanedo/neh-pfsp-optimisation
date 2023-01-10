#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "neh.hpp"

namespace {
using namespace std::literals;

constexpr auto* DATA_PATH = "data";
constexpr auto* INSTANCES_FILENAME = "instances";

auto split(const std::string& str, char delim) {
    auto sstream = std::stringstream{str};
    auto out = std::vector<std::string>{};
    auto token = std::string{};

    while(std::getline(sstream, token, delim)){
        out.push_back(std::move(token));
    }
    return out;
}

auto get_lines(const std::string& filepath) {
  auto istream = std::ifstream{filepath};
  auto out = std::vector<std::string>{};
  auto line = std::string{};

    while(std::getline(istream, line)){
        out.push_back(std::move(line));
    }
    return out;
}

auto read_instance_data(const std::string& filepath) {
  const auto lines = get_lines(filepath);
  const auto dimensions = split(lines[1], ' ');
  const auto number_jobs = std::stoull(dimensions[0]);
  const auto number_machines = std::stoull(dimensions[1]);

  auto processing_times = std::vector<uint64_t>{};
  auto jobs = std::vector<neh::Job<uint64_t>>{};
  jobs.reserve(number_jobs);

  for (size_t i = 0; i < number_jobs; ++i) {
    processing_times.clear();
    auto time_accum = uint64_t{0};

    for (const auto& time_str : split(lines[i + 3], '\t')) {
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
  const auto filename = std::string{INSTANCES_FILENAME};

  for (const auto& instance : get_lines(directory + "/" + filename + ".txt")) {
    auto [jobs, number_jobs, number_machines] = read_instance_data(directory + "/" + instance + ".txt");
    std::cout << "Number of Jobs: " << number_jobs
              << "\nNumber of Machines: " << number_machines << "\n";

    const auto start = std::chrono::high_resolution_clock::now();
    const auto solution = neh::solve(std::move(jobs), number_jobs, number_machines);
    const auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Makespan: " << neh::calculate_makespan(solution)
              << "\nMakespan with Taillard's acceleration: " << solution.makespan
              << "\nElapsed: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
              << "us\n";
    return 0;
  }
}