add_library(esop_fmt INTERFACE)
target_include_directories(esop_fmt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fmt)
target_compile_definitions(esop_fmt INTERFACE FMT_HEADER_ONLY)
