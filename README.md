[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# ESOP Synthesis

## Quick installation guide

Use git to clone the repository

    git clone --recursive https://github.com/hriener/esop_synthesis.git

Add the external dependency `cryptominisat` to the `ext` directory and
proceed with the build instructions of cryptominisat.  Afterwards
build the ESOP synthesis tool as follows:

    mkdir build
    cd build
    cmake ..
    make

## External Dependencies

* [cryptominisat](https://github.com/msoos/cryptominisat)
* [kitty](https://github.com/msoeken/kitty)
* [args](https://github.com/Taywee/args)

## Usage

    ./esop_enum -e --repr=expr <<< "1001"
    
 Output:
    
    [i] synthesize ESOPs for 1001
    1001 2 (~x0*~x1)⊕(x0*x1)
    1001 2 (~x1)⊕(x0)
    1001 2 (~x0)⊕(x1)
    1001 2 (x1)⊕(~x0)
    1001 2 (x0)⊕(~x1)
    1001 2 (x0*x1)⊕(~x0*~x1)
