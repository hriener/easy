add_executable(enum_esops enum_esops.cpp)
target_link_libraries(enum_esops ${EXT_LIBS} easy args kitty json fmt)

add_executable(verify_esops verify_esops.cpp)
target_link_libraries(verify_esops easy args rang kitty)

add_executable(easy_shell easy_shell.cpp)
target_link_libraries(easy_shell ${EXT_LIBS} lorina rang kitty alice fmt cli11 any json easy)
