# Basic simulation ns-3 module

[![Build Status](https://travis-ci.org/snkas/basic-sim.svg?branch=master)](https://travis-ci.org/snkas/basic-sim) [![codecov](https://codecov.io/gh/snkas/basic-sim/branch/master/graph/badge.svg)](https://codecov.io/gh/snkas/basic-sim)

This ns-3 module is intended to make experimental simulation of networks a bit easier. It has a wrapper to take care of loading in run folder configurations (e.g., runtime, random seed), a topology abstraction, an additional routing abstraction called "arbiter routing", a heuristic TCP optimizer, and a few handy applications.

**This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details (in ./LICENSE).**


## Installation

1. Install the following dependencies:
   * Python 3.7+
   * MPI: `sudo apt-get install mpic++`
   * gnuplot (for example plotting): `sudo apt-get install gnuplot`
   * exputilpy (for example plotting): `pip install git+https://github.com/snkas/exputilpy.git`
   * (optional, for testing) lcov: `sudo apt-get install lcov`

2. You need to add the `basic-sim` module into your own ns-3's `contrib/` folder.

    Method 1: git clone
    ```
    cd /path/to/your/folder/of/ns-3/contrib
    git clone https://github.com/snkas/basic-sim.git
    ```
   
    Method 2: git submodule
    ```
    cd /path/to/your/folder/of/ns-3/contrib
    git submodule add https://github.com/snkas/basic-sim.git
    ```
   
    Method 3: download the zip of this git repository and extract it there.
   
3. Now you should be able to compile it along with all your other modules. It has been tested for ns-3 version 3.31.


## Getting started

Documentation (including tutorials) to get started is located in the `doc/` folder.
A good starting point is: `doc/getting_started.md`.


## Testing

Requirements:

* Python 3.6+
* MPI: `sudo apt-get install mpic++`
* lcov: `sudo apt-get install lcov`

To perform the full range of testing of this module:

```
sudo apt-get update
sudo apt-get -y install mpic++
sudo apt-get -y install gnuplot
pip install git+https://github.com/snkas/exputilpy.git
sudo apt-get -y install lcov
cd build
bash build.sh
bash test.sh
bash tutorial.sh
bash example.sh
```


## Acknowledgements

Refactored, extended and maintained by Simon.

Contributions were made by (former) students in the NDAL group who did a project or their thesis, among which: Hussain, Hanjing (list will continue to be updated).

The ECMP routing hashing function is inspired by https://github.com/mkheirkhah/ecmp (accessed: February 20th, 2020), though heavily modified.
