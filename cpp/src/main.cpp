
#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <ratio>
#include <string>

#include "heuristic.h"
#include "inputs.h"
namespace {
using namespace std::literals;

constexpr auto *DATA_PATH = "../data/";
constexpr auto *INSTANCES_FILENAME = "instances.txt";

using DataType = float;
} // namespace

int main() {
    const auto directory = std::string{DATA_PATH};

    for (const auto &instance :
         pfsp::get_lines(directory + INSTANCES_FILENAME)) {
        const auto runs = size_t{1000};
        auto min_time =
                std::chrono::microseconds{std::numeric_limits<int64_t>::max()};
        auto max_time = std::chrono::microseconds{0};
        auto time_accum = std::chrono::microseconds{0};

        std::cout << "Instance name: " << instance << "\n";

        for (size_t i = 0; i != runs; ++i) {
            auto [jobs, number_jobs, number_machines] =
                    pfsp::read_instance_data<DataType, neh::Job<DataType>>(
                            directory + "/" + instance + ".txt");
            const auto [solution, elapsed] =
                    neh::solve(std::move(jobs), number_jobs, number_machines);

            max_time = std::max(max_time, elapsed);
            min_time = std::min(min_time, elapsed);
            time_accum += elapsed;

            if (i == runs - 1) {
                std::cout << "\tNEH makespan: "
                          << neh::calculate_makespan(solution)
                          << "\n\tNEH makespan with Taillard's acceleration: "
                          << solution.makespan << "\n";
            }
        }
        std::cout << "\telapsed avg: " << time_accum.count() / runs << "us\n"
                  << "\telapsed min: " << min_time.count() << "us\n"
                  << "\telapsed max: " << max_time.count() << "us\n";
    }
    return 0;
}