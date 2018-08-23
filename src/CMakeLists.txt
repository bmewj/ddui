list(APPEND ddui_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/ddui.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ddui.cpp
)
add_subdirectory(models)
add_subdirectory(views)
add_subdirectory(util)
set(ddui_SOURCES ${ddui_SOURCES} PARENT_SCOPE)
