#
# Job
#
struct Job
    processing_times::Vector{Int64}
    total_processing_time::Int64
    id::Int64
end

#
# Solution
#
mutable struct Solution
    jobs::Vector{Job}
    number_jobs::Int64
    number_machines::Int64
    makespan::Int64

    function Solution(number_jobs::Int64, number_machines::Int64)
        return new([], number_jobs, number_machines, 0)
    end
end

function populate_e_mat!(solution::Solution, index::Int64, e_mat::Array{Int64})
    e_mat[1, 1] = solution.jobs[1].processing_times[1]
    @inbounds for j in 2:solution.number_machines
        e_mat[1, j] = solution.jobs[1].processing_times[j] + e_mat[1, j-1]
    end
    @inbounds for i in 2:index
        e_mat[i, 1] = solution.jobs[i].processing_times[1] + e_mat[i-1, 1]
        for j in 2:solution.number_machines
            e_mat[i, j] = solution.jobs[i].processing_times[j] + max(e_mat[i-1, j], e_mat[i, j-1])
        end
    end
end

function populate_q_mat!(solution::Solution, index::Int64, q_mat::Array{Int64})
    q_mat[index, :] .= 0
    @inbounds for i in index-1:-1:1, j in solution.number_machines:-1:1
        i_factor = i == index - 1 ? 0 : q_mat[i+1, j]
        j_factor = j == solution.number_machines ? 0 : q_mat[i, j+1]
        q_mat[i, j] = solution.jobs[i].processing_times[j] + max(i_factor, j_factor)
    end
end

function populate_f_mat!(solution::Solution, index::Int64, e_mat::Array{Int64}, f_mat::Array{Int64})
    f_mat[1, 1] = solution.jobs[index].processing_times[1]
    @inbounds for j in 2:solution.number_machines
        f_mat[1, j] = solution.jobs[index].processing_times[j] + f_mat[1, j-1]
    end
    @inbounds for i in 2:index
        f_mat[i, 1] = solution.jobs[index].processing_times[1] + e_mat[i-1, 1]
        @inbounds for j in 2:solution.number_machines
            f_mat[i, j] = solution.jobs[index].processing_times[j] + max(e_mat[i-1, j], f_mat[i, j-1])
        end
    end
end

function try_shift_improve!(solution::Solution, index::Int64, eq_mat::Array{Int64}, f_mat::Array{Int64})
    populate_e_mat!(solution, index, eq_mat)
    populate_f_mat!(solution, index, eq_mat, f_mat)
    populate_q_mat!(solution, index, eq_mat)
    best_index = index
    best_makespan = Inf

    @inbounds for i in 1:index
        max_sum = 0
        @inbounds for j in 1:solution.number_machines
            sum = f_mat[i, j] + eq_mat[i, j]
            if sum > max_sum
                max_sum = sum
            end
        end
        if max_sum < best_makespan
            best_index = i
            best_makespan = max_sum
        end
    end
    if best_index < index
        tmp = solution.jobs[index]
        @inbounds for i = index:-1:best_index+1
            solution.jobs[i] = solution.jobs[i-1]
        end
        solution.jobs[best_index] = tmp
    end
    if index == solution.number_jobs
        solution.makespan = best_makespan
    end
    return best_index
end

function solve_neh!(jobs::Vector{Job}, number_jobs::Int64, number_machines::Int64)
    eq_mat = zeros(Int64, number_jobs, number_machines)
    f_mat = zeros(Int64, number_jobs, number_machines)
    solution = Solution(number_jobs, number_machines)

    sort!(jobs, by=v -> v.total_processing_time, rev=true)
    @inbounds for i in eachindex(jobs)
        push!(solution.jobs, jobs[i])
        try_shift_improve!(solution, i, eq_mat, f_mat)
    end
    return solution
end

function calculate_makespan(solution::Solution)
    times = zeros(Int64, solution.number_jobs, solution.number_machines)
    @inbounds for i in 1:solution.number_jobs
        @inbounds for j in 1:solution.number_machines
            i_factor = i == 1 ? 0 : times[i-1, j]
            j_factor = j == 1 ? 0 : times[i, j-1]
            times[i, j] = solution.jobs[i].processing_times[j] + max(i_factor, j_factor)
        end
    end
    return times[solution.number_jobs, solution.number_machines]
end

function to_string(solution::Solution)
    out = "( "
    for job in solution.jobs
        out = string(out, job.id, " ")
    end
    return string(out, ")")
end