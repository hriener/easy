add_library(experiments INTERFACE)
target_include_directories(experiments INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(experiments INTERFACE fmt json)

file(GLOB FILENAMES *.cpp)

foreach(filename ${FILENAMES})
  get_filename_component(basename ${filename} NAME_WE)
  add_executable(${basename} ${filename})
  target_link_libraries(${basename} PUBLIC experiments easy kitty)
endforeach()
