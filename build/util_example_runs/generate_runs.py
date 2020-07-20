from networkload import *
import exputil

local_shell = exputil.LocalShell()

#########################################################

# Create one central runs directory
local_shell.make_full_dir("runs")

for utilization_interval_ns in [("1s", 1000000000), ("100ms", 100000000), ("10ms", 10000000)]:
    for expected_flows_per_s in [10, 50, 100, 200, 300, 400, 500, 600, 700]:
        run_dir = "runs/util_interval_" + utilization_interval_ns[0] + "_arrival_" + str(expected_flows_per_s)

        # Create run directory
        local_shell.make_full_dir(run_dir)

        # Copy over config_ns3.properties
        local_shell.copy_file("config_ns3.properties", run_dir + "/config_ns3.properties")
        local_shell.sed_replace_in_file_plain(run_dir + "/config_ns3.properties", "[ARRIVAL_RATE]", str(expected_flows_per_s))
        local_shell.sed_replace_in_file_plain(run_dir + "/config_ns3.properties", "[UTILIZATION_INTERVAL]", str(utilization_interval_ns[1]))

        # Core values
        duration_ns = 4 * 1000 * 1000 * 1000  # 4 seconds
        random.seed(123456789)
        seed_start_times = random.randint(0, 100000000)
        seed_flow_size = random.randint(0, 100000000)

        # Start times (ns)
        list_start_time_ns = draw_poisson_inter_arrival_gap_start_times_ns(duration_ns, expected_flows_per_s, seed_start_times)
        num_starts = len(list_start_time_ns)

        # (From, to) tuples
        list_from_to = [(0, 1)] * num_starts

        # Flow sizes in byte
        list_flow_size_byte = list(
            round(x) for x in draw_n_times_from_cdf(num_starts, CDF_PFABRIC_WEB_SEARCH_BYTE, True, seed_flow_size)
        )
        cdf_mean_byte = get_cdf_mean(CDF_PFABRIC_WEB_SEARCH_BYTE, linear_interpolate=True)
        print("Mean: " + str(cdf_mean_byte) + " bytes")
        print("Arrival rate: " + str(expected_flows_per_s) + " flows/s")
        print("Expected utilization: " + str(expected_flows_per_s * cdf_mean_byte / 1.25e+9))

        # Write schedule
        write_schedule(
            run_dir + "/schedule_" + str(expected_flows_per_s) + ".csv",
            num_starts,
            list_from_to,
            list_flow_size_byte,
            list_start_time_ns
        )

        # Write function to run it all
        local_shell.perfect_exec("cd ..; bash run_assist.sh util_example_runs/" + run_dir + " 0", output_redirect=exputil.OutputRedirect.CONSOLE)
