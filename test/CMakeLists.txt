include_directories(catch2) # v2.0.1

file(GLOB_RECURSE FILENAMES *.cpp)

add_executable(run_tests ${FILENAMES})
target_link_libraries(run_tests easy lorina kitty json bill)
if (ENABLE_COVERAGE)
  target_link_libraries(run_tests gcov)
endif()
