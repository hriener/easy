add_executable(enum_esops enum_esops.cpp)
target_link_libraries(enum_esops ${EXT_LIBS} easy easy_args easy_kitty easy_json easy_fmt)

add_executable(verify_esops verify_esops.cpp)
target_link_libraries(verify_esops easy easy_args easy_rang easy_kitty)

add_executable(easy_shell easy_shell.cpp)
target_link_libraries(easy_shell ${EXT_LIBS} easy_lorina easy_rang easy_kitty easy_alice easy_fmt easy_cli11 easy_any easy_json easy)
