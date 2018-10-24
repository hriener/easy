set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -Wall -Wextra")

add_executable(core core.cpp)
target_link_libraries(core ${EXTERNAL_LIBRARIES} easy fmt glucose)

add_executable(enum_esops enum_esops.cpp)
target_link_libraries(enum_esops ${EXTERNAL_LIBRARIES} easy args kitty json fmt)

add_executable(verify_esops verify_esops.cpp)
target_link_libraries(verify_esops easy args rang kitty)

add_executable(easy_shell easy_shell.cpp)
target_link_libraries(easy_shell ${EXTERNAL_LIBRARIES} lorina rang kitty alice fmt cli11 any json easy glucose)
