Installation
============

easy is a header-only C++ library.  Just add the include directory
of easy to your include directories, and you can integrate easy into
your source files using

.. code-block:: c++

   #include <easy/easy.hpp>

Building programs
-----------------

Programs are enabled by default.  In order to build them, run the
following from the base directory of easy::

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
