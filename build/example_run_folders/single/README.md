# Single

1. Simple 0 -- 1 topology with delay of 10000ns, 40 Mbit/s and a
   network device queue size of 100 packets. Queuing disciplines are disabled.
2. Start 1 TCP flows from 0 to 1 which will finish before the end of the simulation
3. Start 1 UDP burst from 0 to 1 at a rate of 15 Mb/s
4. Send pings between them every 50ms
