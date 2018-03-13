list(APPEND ddui_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/TextEdit.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TextEdit.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TextEditModel.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TextEditModel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TextMeasurements.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TextMeasurements.cpp
)
set(ddui_SOURCES ${ddui_SOURCES} PARENT_SCOPE)
