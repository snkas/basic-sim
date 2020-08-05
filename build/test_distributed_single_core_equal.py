import exputil

local_shell = exputil.LocalShell()

# Base normal run folder
base_run_folder = "example_run_folders/leaf_spine"

# Single core runs must produce exactly the same results
for single_core_run_folder in [
    "example_run_folders/leaf_spine_distributed_1_core_default",
    "example_run_folders/leaf_spine_distributed_1_core_default"
]:
    for f in [
        "pingmesh.csv",
        "pingmesh.txt",
        "tcp_flows.csv",
        "tcp_flows.txt",
        "udp_bursts_incoming.csv",
        "udp_bursts_incoming.txt",
        "udp_bursts_outgoing.csv",
        "udp_bursts_outgoing.txt",
        "utilization.csv",
        "utilization_compressed.csv",
        "utilization_compressed.txt",
        "utilization_summary.txt",
    ]:
        # Diff all these files -- if any are different, this will crash because it expects a 0 return from diff
        local_shell.perfect_exec(
            "diff %s/logs_ns3/%s %s/logs_ns3/system_0_%s" % (base_run_folder, f, single_core_run_folder, f),
            output_redirect=exputil.OutputRedirect.CONSOLE
        )
