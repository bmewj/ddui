list(APPEND ddui_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Menu.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Menu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DefaultMenuView.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DefaultMenuView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MenuBuilder.hpp
)
set(ddui_SOURCES ${ddui_SOURCES} PARENT_SCOPE)
