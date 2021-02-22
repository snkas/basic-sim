# Future work

The backlog of additional work that can be done on the `basic-sim` module:

* Update worst-case RTT calculation in topology to include qdisc max. queue size
* Update TCP optimizer to be parameterizable
* Add client remote port selector to the UDP ping scheduler
* Upgrade to the latest available stable version of ns-3
* Make all variable and function naming consistent, along with whitespace conventions,
  in accordance with the ns-3 code style guide

When upgrading to a new version of ns-3, be sure to check:

* `PointToPointAbHelper` still consistent with the code in `PointToPointHelper`
