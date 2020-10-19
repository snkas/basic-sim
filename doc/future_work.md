# Future work

The backlog of additional work that can be done on the `basic-sim` module:

* Incorporate a parameterized selector for RED as queueing discipline
  in the topology configuration default (as RED is commonly used)
  
* Make the TCP optimizer explicitly settable via the configuration

When upgrading to a new version of ns-3, be sure to check:

* `PointToPointAbHelper` still consistent with the code in `PointToPointHelper`

The goal of `basic-sim` is to be a starting module to do ns-3 experiments in
networks. The following is a list of possible extensions, which should not be
part of `basic-sim`, but can be in a new module depending on `basic-sim`:

* Flowlet routing arbiter
* Long-living TCP application and scheduler
* DCTCP example
