[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# ESOP Synthesis

## Quick installation guide

Use git to clone the repository

    git clone https://github.com/hriener/esop_synthesis.git

Add the external extensions cryptominisat, kitty and args to the `ext` directory.

    mkdir build
    cd build
    cmake ..
    make

## External Dependencies

* [cryptominisat](https://github.com/msoos/cryptominisat)
* [kitty](https://github.com/msoeken/kitty)
* [args](https://github.com/Taywee/args)

## Usage

    ./esop_enum -b 1001 -m 1
    
 Output:
    
    [i] compute ESOPs for 1001
    [i] method: SAT-based exact synthesis
    [i] bounded synthesis for k = 1
    [i] bounded synthesis for k = 2
    2 (~x0*~x1)⊕(x0*x1)
    2 (x1)⊕(~x0)
    2 (~x0)⊕(x1)
    2 (x0*x1)⊕(~x0*~x1)
    2 (~x1)⊕(x0)
    2 (x0)⊕(~x1)
