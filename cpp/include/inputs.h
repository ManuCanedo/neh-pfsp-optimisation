#ifndef INPUTS_H
#define INPUTS_H

#include <fstream>
#include <sstream>
#include <tuple>
#include <vector>

namespace pfsp {
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

template <typename DataType, typename JobType>
auto read_instance_data(const std::string &filepath) {
    const auto lines = get_lines(filepath);
    const auto dimensions = split(lines[1], ' ');
    const auto number_jobs = std::stoull(dimensions[0]);
    const auto number_machines = std::stoull(dimensions[1]);

    auto times = std::vector<DataType>{};
    auto jobs = std::vector<JobType>{};
    jobs.reserve(number_jobs);

    for (size_t i = 0; i != number_jobs; ++i) {
        times.clear();
        auto time_accum = DataType{0};

        for (auto &&time_str : split(lines[i + 3], '\t')) {
            const auto time = std::stoull(time_str);
            time_accum += time;
            times.emplace_back(time);
        }
        jobs.emplace_back(std::move(times), time_accum);
    }
    return std::tuple{jobs, number_jobs, number_machines};
}
} // namespace pfsp

#endif // INPUTS_H