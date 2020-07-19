import sys
from exputil import *


def utilization_plot(logs_ns3_dir, data_out_dir, pdf_out_dir, utilization_dir, from_id, to_id):
    local_shell = LocalShell()

    # Read in CSV
    utilization_compressed_csv_columns = read_csv_direct_in_columns(logs_ns3_dir + "/utilization.csv", "pos_int,pos_int,pos_int,pos_int,pos_int")
    num_entries = len(utilization_compressed_csv_columns[0])
    from_list = utilization_compressed_csv_columns[0]
    to_list = utilization_compressed_csv_columns[1]
    range_start_ns_list = utilization_compressed_csv_columns[2]
    range_end_ns_list = utilization_compressed_csv_columns[3]
    busy_ns_list = utilization_compressed_csv_columns[4]

    # Create utilization file
    local_shell.make_full_dir(data_out_dir)
    local_shell.make_full_dir(pdf_out_dir)
    data_filename = data_out_dir + "/utilization_" + str(from_id) + "_to_" + str(to_id)
    with open(data_filename, "w+") as f_out:
        for i in range(num_entries):
            if from_list[i] == from_id and to_list[i] == to_id:
                f_out.write(str(range_start_ns_list[i]) + "," + str(busy_ns_list[i] / (range_end_ns_list[i] - range_start_ns_list[i])) + "\n")
                f_out.write(str(range_end_ns_list[i] - 0.0001) + "," + str(busy_ns_list[i] / (range_end_ns_list[i] - range_start_ns_list[i])) + "\n")

    # Plot time vs. utilization
    pdf_filename = pdf_out_dir + "/plot_utilization_" + str(from_id) + "_to_" + str(to_id) + ".pdf"
    plt_filename = utilization_dir + "/plot_time_vs_utilization.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")


def main():
    args = sys.argv[1:]
    if len(args) != 6:
        print("Must supply exactly six arguments")
        print("Usage: python utilization_plot.py [logs_ns3 directory] [data_out_dir] [pdf_out_dir] [utilization_dir] [from_id] [to_id]")
        exit(1)
    else:
        utilization_plot(
            args[0],
            args[1],
            args[2],
            args[3],
            int(args[4]),
            int(args[5])
        )


if __name__ == "__main__":
    main()
