include_directories(catch2) # v2.0.1

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -Wall -Wextra")

file(GLOB FILENAMES *.cpp algorithm/*.cpp sat/*.cpp)

message(${FILENAMES})

add_executable(run_tests ${FILENAMES})
target_link_libraries(run_tests PUBLIC ${EXT_LIBS} easy lorina kitty json)
