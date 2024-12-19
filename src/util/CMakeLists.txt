list(APPEND ddui_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/draw_text_in_box.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/draw_text_in_box.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/entypo.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/caret_flicker.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/caret_flicker.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/BoxShadow.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/get_asset_filename.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/get_asset_filename.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/get_content_filename.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/open_dialog.hpp
)
add_subdirectory(whereami)

if(APPLE)
    list(APPEND ddui_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/get_content_filename.mm)
    list(APPEND ddui_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/open_dialog.macos.mm)
else()
    list(APPEND ddui_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/get_content_filename.cpp)
    list(APPEND ddui_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/open_dialog.cpp)
endif()

set(ddui_SOURCES ${ddui_SOURCES} PARENT_SCOPE)
