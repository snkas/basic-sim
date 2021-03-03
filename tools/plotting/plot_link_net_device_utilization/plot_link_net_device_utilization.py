import sys
from exputil import *


def plot_link_net_device_utilization(logs_ns3_dir, data_out_dir, pdf_out_dir, from_node_id, to_node_id):
    local_shell = LocalShell()

    # Check that the plotting file is available
    if not local_shell.file_exists("plot_time_vs_link_net_device_utilization.plt"):
        print("The gnuplot file is not present.")
        print("Are you executing this python file inside the plot_link_net_device_utilization directory?")
        exit(1)

    # Create the output directories if they don't exist yet
    local_shell.make_full_dir(data_out_dir)
    local_shell.make_full_dir(pdf_out_dir)

    # Read in CSV
    utilization_compressed_csv_columns = read_csv_direct_in_columns(
        logs_ns3_dir + "/link_net_device_utilization.csv",
        "pos_int,pos_int,pos_int,pos_int,pos_int"
    )
    num_entries = len(utilization_compressed_csv_columns[0])
    from_list = utilization_compressed_csv_columns[0]
    to_list = utilization_compressed_csv_columns[1]
    range_start_ns_list = utilization_compressed_csv_columns[2]
    range_end_ns_list = utilization_compressed_csv_columns[3]
    busy_ns_list = utilization_compressed_csv_columns[4]

    # Create step plottable utilization file with only the changes
    data_filename = "%s/link_net_device_utilization_%d_to_%d_in_intervals.csv" % (
        data_out_dir, from_node_id, to_node_id
    )
    expected_next_start_ns = 0
    with open(data_filename, "w+") as f_out:
        matched = False
        for i in range(num_entries):
            if from_list[i] == from_node_id and to_list[i] == to_node_id:
                matched = True

                # One interval after the other
                if not range_start_ns_list[i] == expected_next_start_ns:
                    raise ValueError("Intervals do not match up: %d vs. %d" % (
                        range_start_ns_list[i],
                        expected_next_start_ns
                    ))

                # Write to file
                utilization_fraction = busy_ns_list[i] / (range_end_ns_list[i] - range_start_ns_list[i])
                f_out.write("%.10f,%.10f\n" % (range_start_ns_list[i], utilization_fraction))
                f_out.write("%.10f,%.10f\n" % (range_end_ns_list[i] - 0.000001, utilization_fraction))

                # Keeping track
                expected_next_start_ns = range_end_ns_list[i]

        # Must find data, else probably invalid node ids
        if not matched:
            raise ValueError("No entries found link %d -> %d" % (from_node_id, to_node_id))

    # Plot time vs. utilization
    pdf_filename = pdf_out_dir + "/plot_link_net_device_utilization_%d_to_%d.pdf" % (from_node_id, to_node_id)
    plt_filename = "plot_time_vs_link_net_device_utilization.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")


def main():
    args = sys.argv[1:]
    if len(args) != 5:
        print("Must supply exactly five arguments")
        print("Usage: python3 plot_link_net_device_utilization.py [logs_ns3 directory] [data_out_dir] [pdf_out_dir]"
              " [from_node_id] [to_node_id]")
        exit(1)
    else:
        plot_link_net_device_utilization(
            args[0],
            args[1],
            args[2],
            int(args[3]),
            int(args[4])
        )


if __name__ == "__main__":
    main()
