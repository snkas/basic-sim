Single everything experiment:

1. Start 1 TCP flows from 0 to 1 starting at t=500ms which will not finish
2. Start 1 UDP burst from 0 to 1 at a rate of 15 Mb/s starting at t=0s
3. Start 1 UDP burst from 1 to 0 at a rate of 19.2 Mb/s starting at t=1s
4. Send pings between them every 50ms
