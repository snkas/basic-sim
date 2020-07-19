import exputil

local_shell = exputil.LocalShell()

# Create one central pdf directory
local_shell.make_full_dir("pdf")

for utilization_interval_ns in [("1s", 1000000000), ("100ms", 100000000), ("10ms", 10000000)]:
    for expected_flows_per_s in [10, 50, 100, 200, 300, 400, 500, 600, 700]:
        run_dir = "runs/util_interval_" + utilization_interval_ns[0] + "_arrival_" + str(expected_flows_per_s)

        # Create utilization plots
        local_shell.perfect_exec(
            "cd ../plot_help/utilization; python utilization_plot.py " +
            "../../util_example_runs/" + run_dir + "/logs_ns3 " +
            "../../util_example_runs/" + run_dir + "/data " +
            "../../util_example_runs/" + run_dir + "/pdf ./ 0 1", output_redirect=exputil.OutputRedirect.CONSOLE)

        # Copy to one central directory
        local_shell.copy_file(run_dir + "/pdf/plot_utilization_0_to_1.pdf", "pdf/util_interval_" + utilization_interval_ns[0] + "_load_" + str(expected_flows_per_s) + ".pdf")
