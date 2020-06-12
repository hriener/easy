cmake_minimum_required(VERSION 3.5.1)
project(easy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(EASY_EXAMPLES    "Build examples"    ON)
option(EASY_TEST        "Build tests"       OFF)
option(EASY_EXPERIMENTS "Build experiments" OFF)
option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" OFF)

if(UNIX)
  # show quite some warnings (but remove some intentionally)
  include(CheckCXXCompilerFlag)
  add_compile_options(-W -Wall -Wextra)
  foreach (WARNING unknown-pragmas gnu-anonymous-struct nested-anon-types)
    check_cxx_compiler_flag("-Wno-${WARNING}" HAS_WNO_${WARNING})
    if (HAS_WNO_${WARNING})
      add_compile_options(-Wno-${WARNING})
    endif()
  endforeach()
  if (ENABLE_COVERAGE)
    add_compile_options(-O0 -g --coverage -fprofile-arcs -ftest-coverage)
  endif()
endif()
if(MSVC)
  add_compile_options(/EHsc /bigobj)
endif()

add_subdirectory(include)
add_subdirectory(lib)

if(EASY_EXAMPLES)
  add_subdirectory(examples)
endif()

if(EASY_TEST)
  add_subdirectory(test)
endif()

if(EASY_EXPERIMENTS)
  add_subdirectory(experiments)
endif()
