import sys
import exputil

from generate_udp_burst_rate_csv import generate_udp_burst_rate_csv


def udp_burst_plot(logs_ns3_dir, data_out_dir, pdf_out_dir, udp_burst_id, interval_ns):
    local_shell = exputil.LocalShell()

    # Check that all plotting files are available
    if not local_shell.file_exists("plot_udp_burst_time_vs_amount_arrived.plt") or \
            not local_shell.file_exists("plot_udp_burst_time_vs_rtt.plt") or \
            not local_shell.file_exists("plot_udp_burst_time_vs_arrival_rate.plt"):
        print("The gnuplot files are not present.")
        print("Are you executing this python file inside the udp_burst_plot directory?")
        exit(1)

    # Create the output directories if they don't exist yet
    local_shell.make_full_dir(data_out_dir)
    local_shell.make_full_dir(pdf_out_dir)

    # Create rate file
    generate_udp_burst_rate_csv(logs_ns3_dir, data_out_dir, udp_burst_id, interval_ns)

    # Plot time vs. amount arrived
    data_filename = logs_ns3_dir + "/udp_burst_" + str(udp_burst_id) + "_amount_arrived.csv"
    local_shell.copy_file(data_filename, data_out_dir + "/udp_burst_" + str(udp_burst_id) + "_amount_arrived.csv")
    pdf_filename = pdf_out_dir + "/plot_udp_burst_time_vs_amount_arrived_" + str(udp_burst_id) + ".pdf"
    plt_filename = "plot_udp_burst_time_vs_amount_arrived.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. rtt
    data_filename = logs_ns3_dir + "/udp_burst_" + str(udp_burst_id) + "_rtt.csv"
    local_shell.copy_file(data_filename, data_out_dir + "/udp_burst" + str(udp_burst_id) + "_rtt.csv")
    pdf_filename = pdf_out_dir + "/plot_udp_burst_time_vs_rtt_" + str(udp_burst_id) + ".pdf"
    plt_filename = "plot_udp_burst_time_vs_rtt.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. arrival rate
    data_filename = data_out_dir + "/udp_burst" + str(udp_burst_id) + "_arrival_rate.csv"
    pdf_filename = pdf_out_dir + "/plot_udp_burst_time_vs_arrival_rate_" + str(udp_burst_id) + ".pdf"
    plt_filename = "plot_udp_burst_time_vs_arrival_rate.plt"
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
        print("Usage: python udp_burst_plot.py [logs_ns3_dir] [data_out_dir] [pdf_out_dir] [udp_burst_id]"
              " [interval_ns (for rates)]")
        exit(1)
    else:
        udp_burst_plot(
            args[0],
            args[1],
            args[2],
            int(args[3]),
            int(args[4])
        )


if __name__ == "__main__":
    main()
