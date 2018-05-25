add_library(easy_fmt INTERFACE)
target_include_directories(easy_fmt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fmt)
target_compile_definitions(easy_fmt INTERFACE FMT_HEADER_ONLY)

add_library(easy_args INTERFACE)
target_include_directories(easy_args INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/args)

add_library(easy_json INTERFACE)
target_include_directories(easy_json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/json/include)

add_library(easy_kitty INTERFACE)
target_include_directories(easy_kitty INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/kitty/include)

add_library(easy_rang INTERFACE)
target_include_directories(easy_rang INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rang)

add_library(easy_lorina INTERFACE)
target_include_directories(easy_lorina INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lorina/include)
target_link_libraries(easy_lorina INTERFACE easy_fmt easy_rang)

add_library(easy_any INTERFACE)
target_include_directories(easy_any INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/any)

add_library(easy_cli11 INTERFACE) # v1.3
target_include_directories(easy_cli11 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cli11)

add_library(easy_alice INTERFACE)
target_include_directories(easy_alice INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/alice/include)
