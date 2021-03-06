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

## Expected outcome: CWND moment of loss

To predict the point at which a loss is observed
you must know two things: the bandwidth-delay product (BDP) and the 
queue size Q of the network device (asserting no queueing discipline exists).

The queue only grows once the bandwidth delay product has been fulfilled.
At some point, the queue will be full, at which point losses will occur.
If the `in-flight size > BDP + Q` there will be a loss. This loss will
be detected after a full round-trip (as then data will be out-of-order).
It will take a full round-trip before this loss is detected via
3 DupACKs. How much will arrive in that round-trip will be `BDP+Q` packets.

### Slow-start

In ns-3, delayed ACK is enabled by default, meaning that for each
two data packets (assuming not a too large time gap between them) a
single ACK packet is sent back. This means that the exponential factor
at slow-start is 1.5x instead of 2x (as CWND is increased for each
ACK packet). With the exponential factor of 1.5x, this results in the
CWND point of loss detection being:

```
CWND when the loss is detected in slow-start = (BDP + Q) * 1.5
```

with: `BDP = link-rate * 2 * link-delay` (2x link-delay because it is RTT)

### Congestion-avoidance

Just like in the slow-start it will take a full RTT of in which a
congestion-avoidance action is executed for all packets of the BDP + Q which
are still in order. However, the goal of congestion-avoidance is to increase
the CWND with one packet per RTT, as such the expected CWND point of loss
in congestion avoidance is:

```
CWND when the loss is detected in congestion avoidance = BDP + Q + 1
```

with: `BDP = link-rate * 2 * link-delay` (2x link-delay because it is RTT)

## What happens when the send (or receive) buffer size is lower than the BDP

If the send buffer size is lower than BDP, it means that the application will
be unable to put enough data into the send buffer to satisfy the full rate
possible. As such, it will continue to operate in the slow-start, in which
every time an ACK is received, the congestion window is increased by one MSS.
As a result, the congestion window will grow very high till it reaches
the ssthresh of MAX_INTEGER. However, the amount of in-flight data will still
only be the total amount that fits in the send buffer.
