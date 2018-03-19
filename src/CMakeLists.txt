list(APPEND ddui_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/app.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Context.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Context.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/keyboard.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/keyboard.cpp
)

if (APPLE)
  list(APPEND ddui_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/app.macos.mm
  )
else ()
  list(APPEND ddui_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/app.linux.cpp
  )
endif ()

add_subdirectory(models)
add_subdirectory(views)
add_subdirectory(util)

set(ddui_SOURCES ${ddui_SOURCES} PARENT_SCOPE)
