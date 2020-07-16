# Basic simulation ns-3 module

[![Build Status](https://travis-ci.org/snkas/basic-sim.svg?branch=master)](https://travis-ci.org/snkas/basic-sim) [![codecov](https://codecov.io/gh/snkas/basic-sim/branch/master/graph/badge.svg)](https://codecov.io/gh/snkas/basic-sim)

This module is used to make the simulation of ns-3 (data center) networks a bit easier. It has a wrapper to take care of loading in run folder configurations (e.g., runtime, random seed), a topology abstraction, an additional routing abstraction called "arbiter routing", a heuristic TCP optimizer, and a few handy applications.

**This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details (in ./LICENSE).**


## Installation

1. You clone or add as git module the `basic-sim` module (this is a dependency for this module) into your ns-3 `src/contrib/` folder:

    ```
    cd /path/to/your/folder/of/ns-3/contrib
    git clone git@github.com:snkas/basic-sim.git
    ```
   
2. Now you should be able to compile it along with all your other modules. It has been tested for ns-3 version 3.30.1.


## Testing

Requirements:

* Python 3.6+
* lcov: `sudo apt-get -y install lcov`

To perform the full range of testing of this module:

```
sudo apt-get update
sudo apt-get -y install lcov
cd build
bash build.sh
bash test.sh
bash example.sh
```


## Acknowledgements

Refactored, extended and maintained by Simon.

Contributions were made by (former) students in the NDAL group who did a project or their thesis, among which: Hussain (list will continue to be updated).

The ECMP routing hashing function is inspired by https://github.com/mkheirkhah/ecmp (accessed: February 20th, 2020), though heavily modified.
