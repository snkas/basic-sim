# Reproduce experiment

The goal of this experiment is to make use of as many features in basic-sim
as possible, such that the logs can be compared if they changed.

1. Topology:  0 -- 1
2. 0 -> 1 has total queue of 50 packets, 0.1% drop rate and rate of 12.5 Mb/s
3. 1 -> 0 has total queue of 50 packets, 0.01% drop rate and rate of 10.25 Mb/s
4. Channel delay of 0 -- 1 is 10 microseconds
5. Start 1 TCP flows from 0 to 1 starting at t=500ms which will not finish
6. Start 1 UDP burst from 0 to 1 at a rate of 7.5 Mb/s starting at t=0s
7. Start 1 UDP burst from 1 to 0 at a rate of 9.6 Mb/s starting at t=1s
8. Send pings between them every 50ms

Expectations:

1. TCP flow achieves a rate of about 12.5 Mb/s - 7.5 Mb/s = 5 Mb/s
2. The UDP pings will vary from 0 till 50 * 1500 byte / 12.5 Mb/s = 48ms approximately
