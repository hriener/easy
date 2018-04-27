add_library(esop_fmt INTERFACE)
target_include_directories(esop_fmt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fmt)
target_compile_definitions(esop_fmt INTERFACE FMT_HEADER_ONLY)

add_library(esop_args INTERFACE)
target_include_directories(esop_args INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/args)

add_library(esop_json INTERFACE)
target_include_directories(esop_json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/json/include)

add_library(esop_kitty INTERFACE)
target_include_directories(esop_kitty INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/kitty/include)

