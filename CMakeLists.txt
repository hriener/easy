cmake_minimum_required(VERSION 3.5.1)
project(easy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Options
option(EASY_EXAMPLES "Build examples"   OFF)
option(EASY_TEST     "Build tests"      OFF)

if((CMAKE_CXX_COMPILER_ID MATCHES GNU) OR (CMAKE_CXX_COMPILER_ID MATCHES Clang))
  set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -Wall -Wextra")
  set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g3 -pg")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

add_subdirectory(lib)

add_library(easy INTERFACE)
target_include_directories(easy INTERFACE ${PROJECT_SOURCE_DIR}/include)

if(EASY_EXAMPLES)
  add_subdirectory(examples)
endif()

if(EASY_TEST)
  add_subdirectory(test)
endif()
