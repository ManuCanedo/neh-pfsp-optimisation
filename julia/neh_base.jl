struct __Job
    id::Int64
    processing_times::Vector{Float64}
    total_processing_time::Float64
end

mutable struct __Solution
    number_jobs::Int64
    number_machines::Int64
    jobs::Vector{__Job}
    makespan::Float64

    function __Solution(number_jobs, number_machines)
        @assert number_jobs > 1
        @assert number_machines > 1
        return new(number_jobs, number_machines, [], 0.0)
    end
end

function __populate_e_mat!(solution, index)
    number_machines = solution.number_machines
    e = [[0.0 for _ = 1:1:number_machines] for _ = 1:1:index]
    for i = 1:1:index
        for j = 1:1:number_machines
            if i == 1 && j == 1
                e[1][1] = solution.jobs[1].processing_times[1]
            elseif j == 1
                e[i][1] = solution.jobs[i].processing_times[1] + e[i-1][1]
            elseif i == 1
                e[1][j] = solution.jobs[1].processing_times[j] + e[1][j-1]
            else
                max_time = max(e[i-1][j], e[i][j-1])
                e[i][j] = solution.jobs[i].processing_times[j] + max_time
            end
        end
    end
    return e
end

function __populate_q_mat!(solution, index)
    number_machines = solution.number_machines
    q = [[0.0 for _ = 1:1:number_machines] for _ = 1:1:index]
    for i = index:-1:1
        for j = number_machines:-1:1
            if i == index
                q[index][j] = 0
            elseif i == index - 1 && j == number_machines
                q[index-1][number_machines] =
                    solution.jobs[index-1].processing_times[number_machines]
            elseif j == number_machines
                q[i][number_machines] =
                    solution.jobs[i].processing_times[number_machines] +
                    q[i+1][number_machines]
            elseif i == index - 1
                q[index-1][j] = solution.jobs[index-1].processing_times[j] + q[index-1][j+1]
            else
                max_time = max(q[i+1][j], q[i][j+1])
                q[i][j] = solution.jobs[i].processing_times[j] + max_time
            end
        end
    end
    return q
end

function __populate_f_mat!(solution, index, e)
    number_machines = solution.number_machines
    f = [[0.0 for _ = 1:1:number_machines] for _ = 1:1:index]
    for i = 1:1:index
        for j = 1:1:number_machines
            if i == 1 && j == 1
                f[1][1] = solution.jobs[index].processing_times[1]
            elseif j == 1
                f[i][1] = solution.jobs[index].processing_times[1] + e[i-1][1]
            elseif i == 1
                f[1][j] = solution.jobs[index].processing_times[j] + f[1][j-1]
            else
                max_time = max(e[i-1][j], f[i][j-1])
                f[i][j] = solution.jobs[index].processing_times[j] + max_time
            end
        end
    end
    return f
end

function __try_shift_improve!(solution, index)
    best_index = index
    solution.makespan = Inf
    e_mat = __populate_e_mat!(solution, index)
    q_mat = __populate_q_mat!(solution, index)
    f_mat = __populate_f_mat!(solution, index, e_mat)

    for i = 1:1:index
        max_sum = 0.0

        for j = 1:1:solution.number_machines
            sum = f_mat[i][j] + q_mat[i][j]
            if sum > max_sum
                max_sum = sum
            end
        end
        new_makespan = max_sum
        if new_makespan < solution.makespan
            best_index = i
            solution.makespan = new_makespan
        end
    end
    if best_index < index
        tmp = solution.jobs[index]

        for i = index:-1:best_index+1
            solution.jobs[i] = solution.jobs[i-1]
        end
        solution.jobs[best_index] = tmp
    end
    return best_index
end

function __solve_neh!(jobs, number_jobs, number_machines)
    solution = __Solution(number_jobs, number_machines)

    start_time = time()

    sort!(jobs, alg = QuickSort, by = v -> v.total_processing_time, rev = true)
    push!(solution.jobs, jobs[1])

    for i = 2:1:number_jobs
        push!(solution.jobs, jobs[i])
        __try_shift_improve!(solution, i)
    end

    end_time = time()
    return solution, (end_time - start_time) * 1000000
end

function __calculate_makespan(solution)
    number_jobs = solution.number_jobs
    number_machines = solution.number_machines
    times = [[0.0 for _ = 1:1:number_machines] for _ = 1:1:number_jobs]
    for j = 1:1:number_machines
        for i = 1:1:number_jobs
            if i == 1 && j == 1
                times[1][1] = solution.jobs[1].processing_times[1]
            elseif j == 1
                times[i][1] = solution.jobs[i].processing_times[1] + times[i-1][1]
            elseif i == 1
                times[1][j] = solution.jobs[1].processing_times[j] + times[1][j-1]
            else
                max_time = max(times[i-1][j], times[i][j-1])
                times[i][j] = solution.jobs[i].processing_times[j] + max_time
            end
        end
    end
    return times[number_jobs][number_machines]
end
