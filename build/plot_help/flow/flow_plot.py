import sys
from exputil import *

try:
    from .generate_rate_txt import generate_rate_txt
except (ImportError, SystemError):
    from generate_rate_txt import generate_rate_txt


def flow_plot(logs_ns3_dir, data_out_dir, pdf_out_dir, flow_dir, interval_ns, flow_id):
    local_shell = LocalShell()
    
    # Create rate file
    generate_rate_txt(logs_ns3_dir, data_out_dir, flow_id, interval_ns)
    
    # Plot time vs. cwnd
    data_filename = logs_ns3_dir + "/flow_" + str(flow_id) + "_cwnd.txt"
    local_shell.copy_file(data_filename, data_out_dir + "/flow_" + str(flow_id) + "_cwnd.txt")
    pdf_filename = pdf_out_dir + "/plot_tcp_time_vs_cwnd_" + str(flow_id) + ".pdf"
    plt_filename = flow_dir + "/plot_tcp_time_vs_cwnd.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. progress
    data_filename = logs_ns3_dir + "/flow_" + str(flow_id) + "_progress.txt"
    local_shell.copy_file(data_filename, data_out_dir + "/flow_" + str(flow_id) + "_progress.txt")
    pdf_filename = pdf_out_dir + "/plot_tcp_time_vs_progress_" + str(flow_id) + ".pdf"
    plt_filename = flow_dir + "/plot_tcp_time_vs_progress.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. rtt
    data_filename = logs_ns3_dir + "/flow_" + str(flow_id) + "_rtt.txt"
    local_shell.copy_file(data_filename, data_out_dir + "/flow_" + str(flow_id) + "_rtt.txt")
    pdf_filename = pdf_out_dir + "/plot_tcp_time_vs_rtt_" + str(flow_id) + ".pdf"
    plt_filename = flow_dir + "/plot_tcp_time_vs_rtt.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. rate
    data_filename = data_out_dir + "/flow_" + str(flow_id) + "_rate.txt"
    pdf_filename = pdf_out_dir + "/plot_tcp_time_vs_rate_" + str(flow_id) + ".pdf"
    plt_filename = flow_dir + "/plot_tcp_time_vs_rate.plt"
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
        print("Usage: python flow_plot.py [logs_ns3 directory] [data_out_dir] [pdf_out_dir] [flow_dir] "
              "[interval_ns] [flow_id]")
        exit(1)
    else:
        flow_plot(
            args[0],
            args[1],
            args[2],
            args[3],
            int(args[4]),
            int(args[5])
        )


if __name__ == "__main__":
    main()
