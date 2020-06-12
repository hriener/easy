add_library(easy INTERFACE)
target_include_directories(easy INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(easy INTERFACE fmt bill rang)
