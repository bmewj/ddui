cmake_minimum_required(VERSION 3.5)

project(ddui C CXX)

list(APPEND ddui_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/gl3w/src/gl3w.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/nanovg/src/nanovg.c 
)

list(APPEND ddui_ASSETS
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/Entypo.ttf
)
set(ddui_ASSETS ${ddui_ASSETS} PARENT_SCOPE)
set(ddui_MACOSX_BUNDLE ${CMAKE_CURRENT_SOURCE_DIR}/assets/MacOSXBundleInfo.plist.in PARENT_SCOPE)

if(NOT DEFINED ddui_GL_VERSION)
    if(APPLE)
        set(ddui_GL_VERSION "GL3")
    else()
        set(ddui_GL_VERSION "GLES3")
    endif()
endif()

add_subdirectory(src)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "build docs")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "build examples")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "build tests")

add_subdirectory(lib/glfw)

set(CMAKE_C_FLAGS "-std=gnu99")

add_library(ddui ${ddui_SOURCES})

target_include_directories(ddui PUBLIC
    include
    lib/nanovg/src
    lib/glfw/include
    lib/gl3w/include
)

if(APPLE)
    list(APPEND EXTRA_LIBS
        "-framework Cocoa -framework OpenGL -framework UniformTypeIdentifiers"
    )
    set(CMAKE_EXE_LINKER_FLAGS "-std=gnu99")

elseif(WIN32)
    list(APPEND EXTRA_LIBS
        "-lglu32 -lopengl32"
    )
    set(CMAKE_EXE_LINKER_FLAGS "-std=gnu99 -static -static-libgcc -static-libstdc++ -mwindows")

else()
    list(APPEND EXTRA_LIBS
        "-lGL -lGLU -lX11"
    )

endif()

target_link_libraries(ddui
    glfw
    whereami
    ${EXTRA_LIBS}
)
