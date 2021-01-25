# Peculiarities

Below are a set of important notes about ns-3 behavior.
Though they might be experienced as bugs, we state them as
peculiarities.

**TCP/UDP**

- A TCP socket when initially Bind()'d to a particular local IP address
  will loose that local IP address upon Connect() and instead will get
  a new one from the route's outgoing interface it is assigned to.

- UDP sockets which did not get Bind()'d to a particular source IP address
  will not have a routing call with the full IP/UDP header, it will only 
  have a call with the IP destination.
