cmake_minimum_required(VERSION 3.5.1)
project(esop)

if((CMAKE_CXX_COMPILER_ID MATCHES GNU) OR (CMAKE_CXX_COMPILER_ID MATCHES Clang))
   set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++14")
   set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g3 -pg")
   set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

include_directories(src)

# external libraries
if (EXISTS ${CMAKE_SOURCE_DIR}/ext/cryptominisat)
  message("Found CRYPTOMINISAT extension")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCRYPTOMINISAT_EXTENSION")
  include_directories(ext/cryptominisat/build/include/)
  link_directories(ext/cryptominisat/build/lib/)
  list(APPEND EXT_LIBS "cryptominisat5.5.0")
endif()

if (EXISTS ${CMAKE_SOURCE_DIR}/ext/kitty)
  message("Found KITTY extension")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DKITTY_EXTENSION")
  include_directories(ext/kitty/include/)
endif()

if (EXISTS ${CMAKE_SOURCE_DIR}/ext/args)
  message("Found ARGS extension")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DARGS_EXTENSION")
  include_directories(ext/args/)
endif()

set(SOURCES
  src/sat/sat_solver.cpp
  src/esop/helliwell.cpp
  src/esop/exact_synthesis.cpp)

add_library(esop_synthesis ${SOURCES})
target_link_libraries(esop_synthesis ${EXT_LIBS})

add_executable(esop_enum programs/esop_enum.cpp)
target_link_libraries(esop_enum esop_synthesis)
