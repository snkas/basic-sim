# TCP one experiment

**Procedure**

1. Simple two nodes with one link in between topology
2. Start one long-lasting TCP flow at T=0

## Expected outcome: TCP rate

Only if the CWND >= BDP is the link-rate reached. As such, the
instantaneous TCP rate is (approximately, excl. header overhead):

```
TCP rate = link-rate * min(1.0, in-flight (byte) / BDP (byte))
```

with: `BDP = link-rate * 2 * link-delay` (2x link-delay because it is RTT) and 
`in-flight = CWND` approximately if there is enough data available, the send/receive buffer
size is large enough and there has not been recent large changes in CWND or the way it is filled.

## Expected outcome: moment of loss

To predict the point at which a loss is observed
you must know two things: the bandwidth-delay product (BDP) and the 
queue size Q of the network device (asserting no queueing discipline exists).
In ns-3, delayed ACK is enabled by default, meaning that for each
two data packets (assuming not a too large time gap between them) a
single ACK packet is sent back. This means that the exponential factor
at slow-start is 1.5x instead of 2x (as CWND is increased for each
ACK packet).

The queue only grows once the bandwidth delay product has been fulfilled.
At some point, the queue will be full, at which point losses will occur.
It will take a full round-trip before this loss is detected via
3 DupACKs. How much will arrive in that round-trip will be `BDP+Q`
With the exponential factor of 1.5x, the results in the
in-flight size point of loss being:

```
In-flight loss point = (BDP + Q) * 1.5
```

with: `BDP = link-rate * 2 * link-delay` (2x link-delay because it is RTT)

If the send buffer size is sufficiently large and enough data available,
the in-flight loss point will equate to the CWND loss point.

## What happens when the send (or receive) buffer size is lower than the BDP

If the send buffer size is lower than BDP, it means that the application will
be unable to put enough data into the send buffer to satisfy the full rate
possible. As such, it will continue to operate in the slow-start, in which
every time an ACK is received, the congestion window is increased by one MSS.
As a result, the congestion window will grow very high till it reaches
the ssthresh of MAX_INTEGER. However, the amount of in-flight data will still
only be the total amount that fits in the send buffer.
