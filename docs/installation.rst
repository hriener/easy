Installation
============

Building easy
-------------

In order to build easy, run the following commands from the base
directory of easy::

  mkdir build
  cd build
  cmake ..
  make

Building tests
--------------

In order to run the tests, you need to enable them in CMake::

  mkdir build
  cd build
  cmake -EASY_TEST=ON ..
  make
  ./test/run_tests
