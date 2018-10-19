cmake_minimum_required(VERSION 3.5.1)
project(easy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Options
option(EASY_BENCH    "Build benchmarks" OFF)
option(EASY_TEST     "Build tests"      OFF)
option(EASY_PROGRAMS "Build programs"   ON)

if((CMAKE_CXX_COMPILER_ID MATCHES GNU) OR (CMAKE_CXX_COMPILER_ID MATCHES Clang))
  set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -Wall -Wextra")
  set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g3 -pg")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

add_subdirectory(lib)

# external libraries
if (EXISTS ${CMAKE_SOURCE_DIR}/lib/cryptominisat)
  message("Found CRYPTOMINISAT extension")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCRYPTOMINISAT_EXTENSION")
  include_directories(lib/cryptominisat/build/include/)
  link_directories(lib/cryptominisat/build/lib/)
  list(APPEND EXTERNAL_LIBRARIES "cryptominisat5.5.0")
endif()

if (EXISTS ${CMAKE_SOURCE_DIR}/lib/glucose-syrup-4.1)
  message("Found GLUCOSE extension")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLUCOSE_EXTENSION")
  add_subdirectory(lib/glucose-syrup-4.1)
  target_compile_options(glucose-lib-static PUBLIC "-w")
  target_compile_options(glucose-lib-shared PUBLIC "-w")
  include_directories(lib/glucose-syrup-4.1/glucose/)
  list(APPEND EXTERNAL_LIBRARIES glucose-lib-static)
endif()

if (EXISTS ${CMAKE_SOURCE_DIR}/lib/breakid-2.3)
  message("Found BREAKID extension")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBREAKID_EXTENSION")
  add_subdirectory(lib/breakid-2.3)
  target_compile_options(breakid-lib-static PUBLIC "-w")
  target_compile_options(breakid-lib-shared PUBLIC "-w")
  include_directories(lib/breakid-2.3/breakid/src/)
  list(APPEND EXTERNAL_LIBRARIES breakid-lib-static)
endif()

add_library(easy INTERFACE)
target_include_directories(easy INTERFACE ${PROJECT_SOURCE_DIR}/include)

if(EASY_PROGRAMS)
  add_subdirectory(programs)
endif()

if(EASY_BENCH)
  add_subdirectory(bench)
endif()

if(EASY_TEST)
  add_subdirectory(test)
endif()
