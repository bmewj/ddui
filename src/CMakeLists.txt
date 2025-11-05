list(APPEND ddui_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/core.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/app.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/app.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/init.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glfw.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/animation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/animation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/timer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/timer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/profiling.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/profiling.cpp
)

if(ddui_BACKEND MATCHES "GL3")
    list(APPEND ddui_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/init.gl3.cpp)
elseif(ddui_BACKEND MATCHES "GLES3")
    list(APPEND ddui_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/init.gles3.cpp)
elseif(ddui_BACKEND MATCHES "D3D11")
    list(APPEND ddui_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/init.d3d11.cpp)
else()
    message(FATAL_ERROR "ddui_BACKEND must be one of: GL3, GLES3, D3D11")
endif()

if(APPLE)
    list(APPEND ddui_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/glfw.macos.mm)
else()
    list(APPEND ddui_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/glfw.cpp)
endif()

add_subdirectory(models)
add_subdirectory(views)
add_subdirectory(util)
set(ddui_SOURCES ${ddui_SOURCES} PARENT_SCOPE)
