[![Build Status](https://travis-ci.org/esop_synthesis/lorina.svg?branch=master)](https://travis-ci.org/hriener/esop_synthesis)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# ESOP Synthesis

## Quick installation guide

Use git to clone the repository

    git clone --recursive https://github.com/hriener/esop_synthesis.git

Afterwards build the ESOP synthesis tool as follows:

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=RELEASE ..
    make

## External dependencies

* [cryptominisat](https://github.com/msoos/cryptominisat)
* [glucose](http://www.labri.fr/perso/lsimon/glucose)
* [kitty](https://github.com/msoeken/kitty)
* [args](https://github.com/Taywee/args)
* [json](https://github.com/nlohmann/json)
* [BreakIt](https://bitbucket.org/krr/breakid)

## Usage

 Generate ESOPs for function `0xcafeaffe`

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

 Generate ESOPs and verify results

    ./enum_esops --repr=cube <<< "0xcafeaffe" | ./verify_esops -e -s

 Output:

    01111111111101010111111101010011 -1000 1--01 1011- 1-0-- 0---- OK
    01111111111101010111111101010011 -1000 1--00 1-00- 1111- ----- OK
    01111111111101010111111101010011 -1000 1--01 1-10- 1111- ----- OK
    01111111111101010111111101010011 -1000 1-100 1-001 1111- ----- OK
    01111111111101010111111101010011 -1000 1--01 1011- --0-- 0-1-- OK
    01111111111101010111111101010011 -1000 1--01 1010- 111-- ----- OK
    01111111111101010111111101010011 -1000 1--01 1011- 1-1-- ----- OK
    [i] total number of errors: 0
