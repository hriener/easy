if (NOT TARGET kitty)
  add_library(kitty INTERFACE) # v0.4
  target_include_directories(kitty INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/kitty)
endif()

if (NOT TARGET sat)
  add_library(sat INTERFACE)
  target_include_directories(sat INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/sat)
endif()
