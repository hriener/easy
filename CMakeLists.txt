cmake_minimum_required(VERSION 3.5.1)
project(easy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

# Options
option(EASY_BENCH "Build benchmarks" OFF)
option(EASY_TEST  "Build tests"      OFF)

if((CMAKE_CXX_COMPILER_ID MATCHES GNU) OR (CMAKE_CXX_COMPILER_ID MATCHES Clang))
  set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -Wall -Wextra")
  set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g3 -pg")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

add_subdirectory(ext)

# external libraries
if (EXISTS ${CMAKE_SOURCE_DIR}/ext/cryptominisat)
  message("Found CRYPTOMINISAT extension")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCRYPTOMINISAT_EXTENSION")
  include_directories(ext/cryptominisat/build/include/)
  link_directories(ext/cryptominisat/build/lib/)
  list(APPEND EXT_LIBS "cryptominisat5.5.0")
endif()

if (EXISTS ${CMAKE_SOURCE_DIR}/ext/glucose-syrup-4.1)
  message("Found GLUCOSE extension")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLUCOSE_EXTENSION")
  add_subdirectory(ext/glucose-syrup-4.1)
  target_compile_options(glucose-lib-static PUBLIC "-w")
  target_compile_options(glucose-lib-shared PUBLIC "-w")
  include_directories(ext/glucose-syrup-4.1/glucose/)
  list(APPEND EXT_LIBS glucose-lib-static)
endif()

if (EXISTS ${CMAKE_SOURCE_DIR}/ext/breakid-2.3)
  message("Found BREAKID extension")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBREAKID_EXTENSION")
  add_subdirectory(ext/breakid-2.3)
  target_compile_options(breakid-lib-static PUBLIC "-w")
  target_compile_options(breakid-lib-shared PUBLIC "-w")
  include_directories(ext/breakid-2.3/breakid/src/)
  list(APPEND EXT_LIBS breakid-lib-static)
endif()

add_library(easy INTERFACE)
target_include_directories(easy INTERFACE ${PROJECT_SOURCE_DIR}/include)

add_subdirectory(programs)

if(EASY_BENCH)
  add_subdirectory(bench)
endif()

if(EASY_TEST)
  add_subdirectory(test)
endif()
