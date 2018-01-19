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

    ./enum_esops -e --repr=expr <<< "0xcafeaffe"
    
 Output:
    
    [i] synthesize ESOPs for 0xcafeaffe
    01111111111101010111111101010011 5 (x1*~x2*~x3*~x4)⊕(x0*~x3*x4)⊕(x0*~x1*x2*x3)⊕(x0*~x2)⊕(~x0)
    01111111111101010111111101010011 5 (x1*~x2*~x3*~x4)⊕(x0*~x3*~x4)⊕(x0*~x2*~x3)⊕(x0*x1*x2*x3)⊕(1)
    01111111111101010111111101010011 5 (x1*~x2*~x3*~x4)⊕(x0*~x3*x4)⊕(x0*x2*~x3)⊕(x0*x1*x2*x3)⊕(1)
    01111111111101010111111101010011 5 (x1*~x2*~x3*~x4)⊕(x0*x2*~x3*~x4)⊕(x0*~x2*~x3*x4)⊕(x0*x1*x2*x3)⊕(1)
    01111111111101010111111101010011 5 (x1*~x2*~x3*~x4)⊕(x0*~x3*x4)⊕(x0*~x1*x2*x3)⊕(~x2)⊕(~x0*x2)
    01111111111101010111111101010011 5 (x1*~x2*~x3*~x4)⊕(x0*~x3*x4)⊕(x0*~x1*x2*~x3)⊕(x0*x1*x2)⊕(1)
    01111111111101010111111101010011 5 (x1*~x2*~x3*~x4)⊕(x0*~x3*x4)⊕(x0*~x1*x2*x3)⊕(x0*x2)⊕(1)
