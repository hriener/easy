[![Build Status](https://travis-ci.org/hriener/esop_synthesis.svg?branch=master)](https://travis-ci.org/hriener/esop_synthesis)
[![Documentation Status](https://readthedocs.org/projects/easy/badge/?version=latest)](http://easy.readthedocs.io/en/latest/?badge=latest)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# easy

easy is a toolkit for finding exclusive-or sum-of-product forms of (incompletely-specified) Boolean functions.

## Installation

Use git to clone the repository

    git clone --recursive https://github.com/hriener/esop_synthesis.git

Build the ESOP synthesis toolkit as follows:

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=RELEASE ..
    make

## Usage

 Generate all minimum ESOP forms for the (completely-specified) Boolean function `0xcafeaffe` (`01111111111101010111111101010011`)

    ./enum_esops --all --echo --repr=expr <<< "0xcafeaffe"
 
 Output:
    
    01111111111101010111111101010011 [...] 5 (x1*~x2*~x3*~x4)⊕(x0*~x3*x4)⊕(x0*~x1*x2*x3)⊕(x0*~x2)⊕(~x0)
    01111111111101010111111101010011 [...] 5 (x1*~x2*~x3*~x4)⊕(x0*~x3*~x4)⊕(x0*~x2*~x3)⊕(x0*x1*x2*x3)⊕(1)
    01111111111101010111111101010011 [...] 5 (x1*~x2*~x3*~x4)⊕(x0*~x3*x4)⊕(x0*x2*~x3)⊕(x0*x1*x2*x3)⊕(1)
    01111111111101010111111101010011 [...] 5 (x1*~x2*~x3*~x4)⊕(x0*x2*~x3*~x4)⊕(x0*~x2*~x3*x4)⊕(x0*x1*x2*x3)⊕(1)
    01111111111101010111111101010011 [...] 5 (x1*~x2*~x3*~x4)⊕(x0*~x3*x4)⊕(x0*~x1*x2*x3)⊕(~x2)⊕(~x0*x2)
    01111111111101010111111101010011 [...] 5 (x1*~x2*~x3*~x4)⊕(x0*~x3*x4)⊕(x0*~x1*x2*~x3)⊕(x0*x1*x2)⊕(1)
    01111111111101010111111101010011 [...] 5 (x1*~x2*~x3*~x4)⊕(x0*~x3*x4)⊕(x0*~x1*x2*x3)⊕(x0*x2)⊕(1)

 Generate all minimum ESOP forms and verify results

    ./enum_esops --all --repr=cube <<< "0xcafeaffe" | ./verify_esops --echo --summary

 Output:

    01111111111101010111111101010011 [...] -1000 1--01 1011- 1-0-- 0---- OK
    01111111111101010111111101010011 [...] -1000 1--00 1-00- 1111- ----- OK
    01111111111101010111111101010011 [...] -1000 1--01 1-10- 1111- ----- OK
    01111111111101010111111101010011 [...] -1000 1-100 1-001 1111- ----- OK
    01111111111101010111111101010011 [...] -1000 1--01 1011- --0-- 0-1-- OK
    01111111111101010111111101010011 [...] -1000 1--01 1010- 111-- ----- OK
    01111111111101010111111101010011 [...] -1000 1--01 1011- 1-1-- ----- OK
    [i] total number of errors: 0

 Generate all minimum ESOP forms for the (incompletely-specified) Boolean function `0xcafe` (`0111111101010011`) with care function `0xaffe` (`0111111111110101`)

    ./enum_esops --all --echo --repr=expr <<< "0xcafe 0xaffe"
 
 Output:
 
    0111111101010011 0111111111110101 3 (~x1*~x2*~x3)⊕(~x0*x2*~x3)⊕(x3)
    0111111101010011 0111111111110101 3 (~x0*~x1*~x2*~x3)⊕(~x0*x2*~x3)⊕(x3)
    0111111101010011 0111111111110101 3 (~x0*x1*x2*~x3)⊕(~x0*~x1*~x3)⊕(x3)
    0111111101010011 0111111111110101 3 (~x0*x1*~x2*~x3)⊕(~x0*~x3)⊕(x3)
    0111111101010011 0111111111110101 3 (x1*~x2*~x3)⊕(~x0*~x3)⊕(x3)
    0111111101010011 0111111111110101 3 (x1*~x2*~x3)⊕(x0*x3)⊕(~x0)
    0111111101010011 0111111111110101 3 (~x0*x1*~x2*~x3)⊕(x0*x3)⊕(~x0)
    0111111101010011 0111111111110101 3 (x1*~x2*~x3)⊕(x0*~x3)⊕(1)
    0111111101010011 0111111111110101 3 (~x0*x1*~x2*~x3)⊕(x0*~x3)⊕(1)
    0111111101010011 0111111111110101 3 (~x0*x1*~x2*~x3)⊕(x0*x2*~x3)⊕(1)
    0111111101010011 0111111111110101 3 (x1*~x2*~x3)⊕(x0*x2*~x3)⊕(1)

## External libraries

* [BreakIt](https://bitbucket.org/krr/breakid)
* [cryptominisat](https://github.com/msoos/cryptominisat)
* [glucose](http://www.labri.fr/perso/lsimon/glucose)
* [kitty](https://github.com/msoeken/kitty.git)
