Introduction
============

Synopsis
--------

easy is a C++ library and toolkit for manipulating and finding exclusive-or sum-of-product (ESOP) forms.  An ESOP form is a classical two-level logic representation of a possibly incompletely-specified multi-output Boolean function.  An ESOP form consisting of one level of multi-fanin AND gates followed by one level of multi-input XOR gates.  ESOP forms have remarkable testability and strong compactness properties;  they play an important role in cryptography and quantum computation.

The easy C++ library provides implementation of logic synthesis and logic optimization algorithms that can be readily integrated with other applications.  Additionally, easy comes with a set of simple tools to synthesize and manipulate ESOP forms.

Tools
-----

* easy_shell
* enum_esop
* verify_esop
