import sys
import exputil
import math
import numpy as np

from generate_tcp_flow_rate_csv import generate_tcp_flow_rate_csv


def plot_tcp_flow(logs_ns3_dir, data_out_dir, pdf_out_dir, tcp_flow_id, interval_ns):
    local_shell = exputil.LocalShell()

    # Check that all plotting files are available
    if not local_shell.file_exists("plot_tcp_flow_time_vs_cwnd.plt") or \
       not local_shell.file_exists("plot_tcp_flow_time_vs_progress.plt") or \
       not local_shell.file_exists("plot_tcp_flow_time_vs_rtt.plt") or \
       not local_shell.file_exists("plot_tcp_flow_time_vs_rate.plt"):
        print("The gnuplot files are not present.")
        print("Are you executing this python file inside the plot_tcp_flow directory?")
        exit(1)

    # Create the output directories if they don't exist yet
    local_shell.make_full_dir(data_out_dir)
    local_shell.make_full_dir(pdf_out_dir)
    
    # Create rate file
    generate_tcp_flow_rate_csv(logs_ns3_dir, data_out_dir, tcp_flow_id, interval_ns)

    # Plot time vs. rate
    data_filename = data_out_dir + "/tcp_flow_" + str(tcp_flow_id) + "_rate_in_intervals.csv"
    pdf_filename = pdf_out_dir + "/plot_tcp_flow_time_vs_rate_" + str(tcp_flow_id) + ".pdf"
    plt_filename = "plot_tcp_flow_time_vs_rate.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. progress
    data_filename = logs_ns3_dir + "/tcp_flow_" + str(tcp_flow_id) + "_progress.csv"
    local_shell.copy_file(data_filename, data_out_dir + "/tcp_flow_" + str(tcp_flow_id) + "_progress.csv")
    pdf_filename = pdf_out_dir + "/plot_tcp_flow_time_vs_progress_" + str(tcp_flow_id) + ".pdf"
    plt_filename = "plot_tcp_flow_time_vs_progress.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. rtt
    data_filename = logs_ns3_dir + "/tcp_flow_" + str(tcp_flow_id) + "_rtt.csv"
    local_shell.copy_file(data_filename, data_out_dir + "/tcp_flow_" + str(tcp_flow_id) + "_rtt.csv")
    pdf_filename = pdf_out_dir + "/plot_tcp_flow_time_vs_rtt_" + str(tcp_flow_id) + ".pdf"
    plt_filename = "plot_tcp_flow_time_vs_rtt.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. rto
    data_filename = logs_ns3_dir + "/tcp_flow_" + str(tcp_flow_id) + "_rto.csv"
    local_shell.copy_file(data_filename, data_out_dir + "/tcp_flow_" + str(tcp_flow_id) + "_rto.csv")
    pdf_filename = pdf_out_dir + "/plot_tcp_flow_time_vs_rto_" + str(tcp_flow_id) + ".pdf"
    plt_filename = "plot_tcp_flow_time_vs_rto.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. cwnd
    data_filename = logs_ns3_dir + "/tcp_flow_" + str(tcp_flow_id) + "_cwnd.csv"
    local_shell.copy_file(data_filename, data_out_dir + "/tcp_flow_" + str(tcp_flow_id) + "_cwnd.csv")
    pdf_filename = pdf_out_dir + "/plot_tcp_flow_time_vs_cwnd_" + str(tcp_flow_id) + ".pdf"
    plt_filename = "plot_tcp_flow_time_vs_cwnd.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. cwnd_inflated
    data_filename = logs_ns3_dir + "/tcp_flow_" + str(tcp_flow_id) + "_cwnd_inflated.csv"
    local_shell.copy_file(data_filename, data_out_dir + "/tcp_flow_" + str(tcp_flow_id) + "_cwnd_inflated.csv")
    pdf_filename = pdf_out_dir + "/plot_tcp_flow_time_vs_cwnd_inflated_" + str(tcp_flow_id) + ".pdf"
    plt_filename = "plot_tcp_flow_time_vs_cwnd_inflated.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. ssthresh
    data_filename = logs_ns3_dir + "/tcp_flow_" + str(tcp_flow_id) + "_ssthresh.csv"

    # Retrieve the highest ssthresh which is not a max. integer
    ssthresh_values = exputil.read_csv_direct_in_columns(data_filename, "pos_int,pos_int,pos_int")[2]
    max_ssthresh = 0
    for ssthresh in ssthresh_values:
        if ssthresh > max_ssthresh and ssthresh != 4294967295:
            max_ssthresh = ssthresh
    if max_ssthresh == 0:  # If it never got out of initial slow-start, we just set it to 1 for the plot
        max_ssthresh = 1.0

    # Execute ssthresh plotting
    local_shell.copy_file(data_filename, data_out_dir + "/tcp_flow_" + str(tcp_flow_id) + "_ssthresh.csv")
    pdf_filename = pdf_out_dir + "/plot_tcp_flow_time_vs_ssthresh_" + str(tcp_flow_id) + ".pdf"
    plt_filename = "plot_tcp_flow_time_vs_ssthresh.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[MAX-Y]", str(math.ceil(max_ssthresh / 1380.0)))
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. inflight
    data_filename = logs_ns3_dir + "/tcp_flow_" + str(tcp_flow_id) + "_inflight.csv"
    local_shell.copy_file(data_filename, data_out_dir + "/tcp_flow_" + str(tcp_flow_id) + "_inflight.csv")
    pdf_filename = pdf_out_dir + "/plot_tcp_flow_time_vs_inflight_" + str(tcp_flow_id) + ".pdf"
    plt_filename = "plot_tcp_flow_time_vs_inflight.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. together (cwnd, cwnd_inflated, ssthresh, inflight)
    cwnd_values = exputil.read_csv_direct_in_columns(
        logs_ns3_dir + "/tcp_flow_" + str(tcp_flow_id) + "_cwnd.csv", "pos_int,pos_int,pos_int")[2]
    cwnd_inflated_values = exputil.read_csv_direct_in_columns(
        logs_ns3_dir + "/tcp_flow_" + str(tcp_flow_id) + "_cwnd_inflated.csv", "pos_int,pos_int,pos_int")[2]
    inflight_values = exputil.read_csv_direct_in_columns(
        logs_ns3_dir + "/tcp_flow_" + str(tcp_flow_id) + "_inflight.csv", "pos_int,pos_int,pos_int")[2]
    pdf_filename = pdf_out_dir + "/plot_tcp_flow_time_vs_together_" + str(tcp_flow_id) + ".pdf"
    plt_filename = "plot_tcp_flow_time_vs_together.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[MAX-Y]", str(
        max(
            math.ceil(max_ssthresh / 1380.0),
            math.ceil(np.max(cwnd_values) / 1380.0),
            math.ceil(np.max(cwnd_inflated_values) / 1380.0),
            math.ceil(np.max(inflight_values) / 1380.0)
        )
    ))
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain(
        "temp.plt", "[DATA-FILE-CWND]", logs_ns3_dir + "/tcp_flow_" + str(tcp_flow_id) + "_cwnd.csv"
    )
    local_shell.sed_replace_in_file_plain(
        "temp.plt", "[DATA-FILE-CWND-INFLATED]", logs_ns3_dir + "/tcp_flow_" + str(tcp_flow_id) + "_cwnd_inflated.csv"
    )
    local_shell.sed_replace_in_file_plain(
        "temp.plt", "[DATA-FILE-SSTHRESH]", logs_ns3_dir + "/tcp_flow_" + str(tcp_flow_id) + "_ssthresh.csv"
    )
    local_shell.sed_replace_in_file_plain(
        "temp.plt", "[DATA-FILE-INFLIGHT]", logs_ns3_dir + "/tcp_flow_" + str(tcp_flow_id) + "_inflight.csv"
    )
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")


def main():
    args = sys.argv[1:]
    if len(args) != 5:
        print("Must supply exactly five arguments")
        print("Usage: python3 plot_tcp_flow.py [logs_ns3_dir] [data_out_dir] [pdf_out_dir] [tcp_flow_id]"
              " [interval_ns (for rates)]")
        exit(1)
    else:
        plot_tcp_flow(
            args[0],
            args[1],
            args[2],
            int(args[3]),
            int(args[4])
        )


if __name__ == "__main__":
    main()
