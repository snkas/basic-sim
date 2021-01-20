with open("udp_ping_schedule.csv", "w+") as f_out:
    n = 0
    for i in range(0, 25):
        for j in range(0, 25):
            if i != j:
                f_out.write("%d,%d,%d,100000000,0,100000000000,0,,\n" % (n, i, j))
                n += 1