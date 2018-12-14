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
    ${CMAKE_CURRENT_SOURCE_DIR}/profiling.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/profiling.cpp
)

if(ddui_GL_VERSION MATCHES "GL3")
    list(APPEND ddui_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/init.gl3.cpp)
elseif(ddui_GL_VERSION MATCHES "GLES3")
    list(APPEND ddui_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/init.gles3.cpp)
else()
    message(FATAL_ERROR "ddui_GL_VERSION must be one of: GL3, GLES3")
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
