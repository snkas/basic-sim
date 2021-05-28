Side-by-side experiment:

1. Start 2 TCP flows from 0 to 1
2. UDP burst happening from 0 -> 1 at 30 Mb/s, and 3 -> 2 at 20 Mb/s
3. Pings are sent in a 50ms interval from 0 to 1, and 3 to 2

The distributed versions of this experiment should yield the same results.
