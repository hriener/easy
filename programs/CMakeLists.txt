add_executable(enum_esops enum_esops.cpp)
target_link_libraries(enum_esops easy easy_args)

add_executable(verify_esops verify_esops.cpp)
target_link_libraries(verify_esops easy easy_args easy_rang)

add_executable(esop_maniac esop_maniac.cpp)
target_link_libraries(esop_maniac easy easy_args easy_rang easy_lorina)

add_executable(easy_shell easy_shell.cpp)
target_link_libraries(easy_shell easy_alice easy_fmt easy_cli11 easy_any easy_json easy)
