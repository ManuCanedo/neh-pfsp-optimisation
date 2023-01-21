struct Job
    processing_times::Vector{Int64}
    total_processing_time::Int64
    id::Int64
end

mutable struct Solution
    jobs::Vector{Job}
    number_jobs::Int64
    number_machines::Int64
    makespan::Int64

    function Solution(number_jobs::Int64, number_machines::Int64)
        @assert number_jobs > 1
        @assert number_machines > 1
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
    @inbounds for j in solution.number_machines:-1:1
        q_mat[index, j] = 0
    end
    if index == 1
        return
    end
    q_mat[index-1, solution.number_machines] = solution.jobs[index-1].processing_times[solution.number_machines]

    @inbounds for j in solution.number_machines-1:-1:1
        q_mat[index-1, j] = solution.jobs[index-1].processing_times[j] + q_mat[index-1, j+1]
    end
    if index == 2
        return
    end
    @inbounds for i in index-2:-1:1
        q_mat[i, solution.number_machines] = solution.jobs[i].processing_times[solution.number_machines] + q_mat[i+1, solution.number_machines]

        @inbounds for j in solution.number_machines-1:-1:1
            q_mat[i, j] = solution.jobs[i].processing_times[j] + max(q_mat[i+1, j], q_mat[i, j+1])
        end
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
    solution.makespan = typemax(Int64)

    @inbounds for i in 1:index
        max_sum = 0

        @inbounds for j in 1:solution.number_machines
            sum = f_mat[i, j] + eq_mat[i, j]
            if sum > max_sum
                max_sum = sum
            end
        end
        if max_sum < solution.makespan
            best_index = i
            solution.makespan = max_sum
        end
    end
    if best_index < index
        tmp = solution.jobs[index]

        @inbounds for i in index:-1:best_index+1
            solution.jobs[i] = solution.jobs[i-1]
        end
        solution.jobs[best_index] = tmp
    end
    return best_index
end

function solve_neh!(jobs::Vector{Job}, number_jobs::Int64, number_machines::Int64)
    eq_mat = zeros(Int64, number_jobs, number_machines)
    f_mat = zeros(Int64, number_jobs, number_machines)
    solution = Solution(number_jobs, number_machines)

    start_time = time()

    sort!(jobs, alg=QuickSort, by=v -> v.total_processing_time, rev=true)
    push!(solution.jobs, jobs[1])

    @inbounds for i in 2:number_jobs
        push!(solution.jobs, jobs[i])
        try_shift_improve!(solution, i, eq_mat, f_mat)
    end

    end_time = time()
    return solution, (end_time - start_time) * 1000000
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