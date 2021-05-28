# Future work

The backlog of additional work that can be done on the `basic-sim` module:

* Add client remote port selector to the UDP ping scheduler
* Upgrade to the latest available stable version of ns-3
* Make all variable and function naming consistent, along with whitespace conventions,
  in accordance with the ns-3 code style guide
* Investigate further TCP behavior with multiple logical processes (systems)

When upgrading to a new version of ns-3, be sure to check:

* `PointToPointAbHelper` still consistent with the code in `PointToPointHelper`
