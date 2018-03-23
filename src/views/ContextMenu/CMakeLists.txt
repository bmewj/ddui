list(APPEND ddui_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/ContextMenu.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ContextMenuState.hpp
)

if(APPLE)
  list(APPEND ddui_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/ContextMenu.macos.mm
  )
else()
  list(APPEND ddui_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/ContextMenu.cpp
  )
endif()

set(ddui_SOURCES ${ddui_SOURCES} PARENT_SCOPE)
