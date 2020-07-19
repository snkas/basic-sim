import sys


def generate_rate_txt(logs_ns3_dir, data_out_dir, flow_id, interval_ns):

    # Calculate rate in windows
    with open(data_out_dir + "/flow_" + str(flow_id) + "_rate.txt", "w+") as f_rate:
        with open(logs_ns3_dir + "/flow_" + str(flow_id) + "_progress.txt", "r") as f_in:
            last_update_ns = 0
            last_progress_byte = 0
            for line in f_in:
                spl = line.split(",")
                line_flow_id = int(spl[0])
                line_time_ns = int(spl[1])
                line_progress_byte = int(spl[2])
                if line_time_ns > last_update_ns + interval_ns:
                    f_rate.write("%d,%d,%.2f\n" % (
                        line_flow_id,
                        last_update_ns + 0.00001,
                        float(line_progress_byte - last_progress_byte) / float(line_time_ns - last_update_ns) * 8000.0)
                    )
                    f_rate.write("%d,%d,%.2f\n" % (
                        line_flow_id,
                        line_time_ns,
                        float(line_progress_byte - last_progress_byte) / float(line_time_ns - last_update_ns) * 8000.0)
                                 )
                    last_update_ns = line_time_ns
                    last_progress_byte = line_progress_byte

    print("Interval: " + str(interval_ns / 1000000.0) + " ms")
    print("Line format: [flow_id],[time_moment_ns],[rate in Mbps]")
    print("Produced: " + logs_ns3_dir + "/flow_" + str(flow_id) + "_rate.txt")


if __name__ == "__main__":
    args = sys.argv[1:]
    if len(args) != 3:
        print("Must supply exactly three arguments")
        print("Usage: python generate_rate_txt.py [logs_ns3 directory] [flow_id] [interval in ns (int)]")
        exit(1)
    else:
        generate_rate_txt(
            args[0],
            args[0],
            int(args[1]),
            int(args[2])
        )
