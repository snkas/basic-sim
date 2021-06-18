# Basic simulation ns-3 module

[![build](https://github.com/snkas/basic-sim/workflows/build/badge.svg)](https://github.com/snkas/basic-sim/actions?query=workflow%3Abuild+branch%3Amaster)
[![codecov](https://codecov.io/gh/snkas/basic-sim/branch/master/graph/badge.svg)](https://codecov.io/gh/snkas/basic-sim)
[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://img.shields.io/badge/License-GPL%20v2-blue.svg)

This ns-3 module is intended to make experimental simulation of networks a bit easier.
It has a wrapper to take care of loading in run folder configurations (e.g., runtime, random seed),
a topology abstraction, an additional routing abstraction called "arbiter routing",
a TCP configuration helper, and three handy applications (TCP flows, UDP bursts, and UDP pings).

**This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details (in ./LICENSE).**


## Installation

The below instructions assume you already have a working copy of ns-3 located at 
`/path/to/your/folder/of/ns-3`.

1. **Dependencies**
   
   Install the following dependencies:

   * Python 3.7+
   * MPI:
     ```
     sudo apt-get install openmpi-bin openmpi-common openmpi-doc libopenmpi-dev
     ```
   * lcov: `sudo apt-get install lcov`
   
   Additionally, if you are going to make use of functionality beyond the C++ ns-3 module:
   
   * gnuplot (for plotting): `sudo apt-get install gnuplot`
   * numpy (for plotting): `pip3 install numpy`
   * statsmodels (for plotting): `pip3 install statsmodels`
   * exputilpy (for testing and plotting): `pip3 install git+https://github.com/snkas/exputilpy.git`

2. **Add the basic-sim module**

   You need to add the `basic-sim` module into your own ns-3's `contrib/` folder.

    Method 1: git clone (strongly recommended to enforce a particular commit checkout to manage the version you use)
    ```
    cd /path/to/your/folder/of/ns-3/contrib
    git clone https://github.com/snkas/basic-sim.git
    cd basic-sim
    git checkout <commit-sha>
    ```
    
    Method 2: git submodule
    ```
    cd /path/to/your/folder/of/ns-3/contrib
    git submodule add https://github.com/snkas/basic-sim.git
    ```
   
    Method 3: manually download the zip of this git repository and extract it there.
   
3. **Compile**

   Now you should be able to compile it along with all your other modules.
   It has been tested for ns-3 version 3.33.
   ```
   cd /path/to/your/folder/of/ns-3
   ./waf configure
   ./waf
   ```


## Getting started

Documentation (including tutorials) to get started is located in the `doc/` folder.
A good starting point is: `doc/getting_started.md`.


## Testing

To perform the full range of testing of this module (Python 3.7+):

```
sudo apt-get update
sudo apt-get -y install openmpi-bin openmpi-common openmpi-doc libopenmpi-dev
sudo apt-get -y install lcov
sudo apt-get -y install gnuplot
pip3 install numpy
pip3 install statsmodels
pip3 install git+https://github.com/snkas/exputilpy.git
cd build
bash build.sh
bash test.sh
bash tutorial.sh
bash example_main.sh
bash example_programmable.sh
```


## Acknowledgements

Refactored, extended and maintained by Simon.

Contributions were made by (former) students who collaborated with me, among which:

* Hussain
* Hanjing 
* (list will continue to be updated)

Several gnuplot files in `tools/plotting` are under the MIT license.
They make use of a gnuplot (styling) released under the MIT license.
See `tools/plotting/MIT_LICENSE` for more information including a full list of its authors.
