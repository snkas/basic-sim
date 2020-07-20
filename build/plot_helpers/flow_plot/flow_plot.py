import sys
import exputil

from generate_flow_rate_csv import generate_flow_rate_csv


def flow_plot(logs_ns3_dir, data_out_dir, pdf_out_dir, flow_id, interval_ns):
    local_shell = exputil.LocalShell()

    # Check that all plotting files are available
    if not local_shell.file_exists("plot_tcp_time_vs_cwnd.plt") or \
       not local_shell.file_exists("plot_tcp_time_vs_progress.plt") or \
       not local_shell.file_exists("plot_tcp_time_vs_rtt.plt") or \
       not local_shell.file_exists("plot_tcp_time_vs_rate.plt"):
        print("The gnuplot files are not present.")
        print("Are you executing this python file inside the flow_plot directory?")
        exit(1)

    # Create the output directories if they don't exist yet
    local_shell.make_full_dir(data_out_dir)
    local_shell.make_full_dir(pdf_out_dir)
    
    # Create rate file
    generate_flow_rate_csv(logs_ns3_dir, data_out_dir, flow_id, interval_ns)
    
    # Plot time vs. cwnd
    data_filename = logs_ns3_dir + "/flow_" + str(flow_id) + "_cwnd.csv"
    local_shell.copy_file(data_filename, data_out_dir + "/flow_" + str(flow_id) + "_cwnd.csv")
    pdf_filename = pdf_out_dir + "/plot_tcp_time_vs_cwnd_" + str(flow_id) + ".pdf"
    plt_filename = "plot_tcp_time_vs_cwnd.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. progress
    data_filename = logs_ns3_dir + "/flow_" + str(flow_id) + "_progress.csv"
    local_shell.copy_file(data_filename, data_out_dir + "/flow_" + str(flow_id) + "_progress.csv")
    pdf_filename = pdf_out_dir + "/plot_tcp_time_vs_progress_" + str(flow_id) + ".pdf"
    plt_filename = "plot_tcp_time_vs_progress.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. rtt
    data_filename = logs_ns3_dir + "/flow_" + str(flow_id) + "_rtt.csv"
    local_shell.copy_file(data_filename, data_out_dir + "/flow_" + str(flow_id) + "_rtt.csv")
    pdf_filename = pdf_out_dir + "/plot_tcp_time_vs_rtt_" + str(flow_id) + ".pdf"
    plt_filename = "plot_tcp_time_vs_rtt.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. rate
    data_filename = data_out_dir + "/flow_" + str(flow_id) + "_rate.csv"
    pdf_filename = pdf_out_dir + "/plot_tcp_time_vs_rate_" + str(flow_id) + ".pdf"
    plt_filename = "plot_tcp_time_vs_rate.plt"
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
        print("Usage: python flow_plot.py [logs_ns3_dir] [data_out_dir] [pdf_out_dir] [flow_id]"
              " [interval_ns (for rates)]")
        exit(1)
    else:
        flow_plot(
            args[0],
            args[1],
            args[2],
            int(args[3]),
            int(args[4])
        )


if __name__ == "__main__":
    main()
