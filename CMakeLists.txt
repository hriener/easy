cmake_minimum_required(VERSION 3.5.1)
project(esop LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

# Options
option(ESOP_TEST "Build tests" OFF)

if((CMAKE_CXX_COMPILER_ID MATCHES GNU) OR (CMAKE_CXX_COMPILER_ID MATCHES Clang))
   set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -Wall -Wextra")
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

if (EXISTS ${CMAKE_SOURCE_DIR}/ext/json)
  message("Found JSON extension")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DJSON_EXTENSION")
  include_directories(ext/json/include/)
endif()

set(SOURCES
  src/utils/string_utils.cpp
  src/sat/sat_solver.cpp
  src/sat/cnf_writer.cpp
  src/sat/gauss.cpp
  src/sat/xor_clauses_to_cnf.cpp
  src/sat/cnf_symmetry_breaking.cpp
  src/esop/print.cpp
  src/esop/helliwell.cpp
  src/esop/combine.cpp
  src/esop/cube_utils.cpp
  src/esop/exact_synthesis.cpp)

add_library(esop_synthesis ${SOURCES})
target_link_libraries(esop_synthesis ${EXT_LIBS})

add_executable(enum_esops programs/enum_esops.cpp)
target_link_libraries(enum_esops esop_synthesis)

add_executable(verify_esops programs/verify_esops.cpp)
target_link_libraries(verify_esops esop_synthesis)

if(ESOP_TEST)
  add_subdirectory(test)
endif()
