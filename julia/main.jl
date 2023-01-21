include("neh.jl")
include("neh_base.jl")

function read_instance_data(directory::String, filename::String)
    filepath = "$directory/$filename.txt"
    lines = readlines(filepath)
    number_jobs, number_machines = map(x -> parse(Int64, x), split(lines[2], r"[' '\t]+"))
    jobs = []
    for i = 1:number_jobs
        processing_times = []
        time_accum = 0
        for (_, time_str) in enumerate(split(lines[i+3], "\t"))
            time = parse(Int64, time_str)
            time_accum += time
            push!(processing_times, time)
        end
        push!(jobs, Job(Vector{Int64}(processing_times), time_accum, i - 1))
    end

    return Vector{Job}(jobs), number_jobs, number_machines
end

function __read_instance_data(directory::String, filename::String)
    filepath = "$directory/$filename.txt"
    lines = readlines(filepath)
    number_jobs, number_machines = map(x -> parse(Int64, x), split(lines[2], r"[' '\t]+"))
    __jobs = []
    for i = 1:number_jobs
        processing_times = []
        time_accum = 0
        for (_, time_str) in enumerate(split(lines[i+3], "\t"))
            time = parse(Int64, time_str)
            time_accum += time
            push!(processing_times, time)
        end
        push!(__jobs, __Job(i - 1, processing_times, time_accum))
    end

    return __jobs, number_jobs, number_machines
end

function read_instances(directory::String, filename::String)
    return readlines("$directory/$filename.txt")
end

function main()
    directory = "data"
    instances_filename = "instances"
    instances = read_instances(directory, instances_filename)

    for instance in instances
        runs = 1000
        min_time = Inf
        max_time = 0
        elapsed_accum = 0
        println("Instance name: ", instance)
        println("\tBase implementation:")

        for i = 1:runs
            __jobs, number_jobs, number_machines = __read_instance_data(directory, instance)
            __solution, elapsed = __solve_neh!(__jobs, number_jobs, number_machines)

            min_time = min(min_time, elapsed)
            max_time = max(max_time, elapsed)
            elapsed_accum += elapsed

            if i == runs
                println("\t\tNEH makespan: ", __calculate_makespan(__solution))
                println(
                    "\t\tNEH makespan with Taillard's acceleration: ",
                    __solution.makespan,
                )
            end
        end
        println("\t\telapsed avg: ", elapsed_accum / runs, "us")
        println("\t\telapsed min: ", min_time, "us")
        println("\t\telapsed max: ", max_time, "us")

        min_time = Inf
        max_time = 0
        elapsed_accum = 0

        println("\tOptimised implementation:")
        for i = 1:runs
            jobs, number_jobs, number_machines = read_instance_data(directory, instance)
            solution, elapsed = solve_neh!(jobs, number_jobs, number_machines)

            min_time = min(min_time, elapsed)
            max_time = max(max_time, elapsed)
            elapsed_accum += elapsed

            if i == runs
                println("\t\tNEH makespan: ", calculate_makespan(solution))
                println(
                    "\t\tNEH makespan with Taillard's acceleration: ",
                    solution.makespan,
                )
            end
        end
        println("\t\telapsed avg: ", elapsed_accum / runs, "us")
        println("\t\telapsed min: ", min_time, "us")
        println("\t\telapsed max: ", max_time, "us")

    end
end

main()
