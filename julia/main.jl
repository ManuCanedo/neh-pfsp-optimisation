include("neh.jl")

function read_instance_data(directory::String, filename::String)
    filepath = "$directory/$filename.txt"
    lines = readlines(filepath)
    number_jobs, number_machines = map(x -> parse(Int64, x), split(lines[2], r"[' '\t]+"))
    jobs = []
    for i in 1:number_jobs
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

function read_instances(directory::String, filename::String)
    return readlines("$directory/$filename.txt")
end

function main()
    directory = "data"
    instances_filename = "instances"
    instances = read_instances(directory, instances_filename)

    for instance in instances
        runs = 200
        min_time = Inf
        max_time = 0
        elapsed_accum = 0

        println("Instance name: ", instance)

        for i in 1:runs
            jobs, number_jobs, number_machines = read_instance_data(directory, instance)
            solution, elapsed = solve_neh!(jobs, number_jobs, number_machines)

            min_time = min(min_time, elapsed)
            max_time = max(max_time, elapsed)
            elapsed_accum += elapsed

            if i == runs
                println("Number of Jobs: ", number_jobs, "\nNumber of Machines: ", number_machines)
                println("NEH makespan: ", calculate_makespan(solution))
                println("NEH makespan with Taillard's acceleration: ", solution.makespan)
            end
        end
        println("elapsed avg: ", elapsed_accum / runs * 1000)
        println("elapsed min: ", min_time * 1000)
        println("elapsed max: ", max_time * 1000)
    end
end

main()