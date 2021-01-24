# Future work

The backlog of additional work that can be done on the `basic-sim` module:

* Add test for ECN marking for the simple_red queueing discipline
* Upgrade to latest ns-3
* Make all variable and function naming consistent, along with whitespace conventions,
  in accordance with the ns-3 code style guide

When upgrading to a new version of ns-3, be sure to check:

* `PointToPointAbHelper` still consistent with the code in `PointToPointHelper`

The goal of `basic-sim` is to be a starting module to do ns-3 experiments in
networks. The following is a list of possible extensions, which should not be
part of `basic-sim`, but can be in a new module depending on `basic-sim`:

* Flowlet routing arbiter
* Long-living TCP application and scheduler
* DCTCP example
