list(APPEND ddui_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/app.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Context.hpp
)
add_subdirectory(views)
set(ddui_SOURCES ${ddui_SOURCES} PARENT_SCOPE)
