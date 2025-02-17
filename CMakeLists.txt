# Invoking the CMake build from the command line is a two step process, first, *generate* a build by running the following:
#
#   cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Release
# 
# Where `Release` can be replaced with `Debug`, which contains gdb and address sanitiser definitions already written for you.
# Once a build is created, proceed with *compilation*:
# 
#   cmake --build build --target <project name> --config Release # optionally, for parallel build, append -j $(nproc)
#
# This creates the executable in the build directory. You only need to *generate* a build if you modify the CMakeList.txt file.
# For any other changes to the source code, simply recompile.


cmake_minimum_required(VERSION 3.12)
project(Renderer)

FILE(GLOB_RECURSE ALL_SOURCES src/*.cpp)
FILE(GLOB_RECURSE ALL_HEADERS src/*.h)

set(MAIN_SOURCE src/main.cpp)
set(CMAKE_CXX_FLAGS -fdiagnostics-color=always)

set(CMAKE_CXX_STANDARD 20)

# Note, we do this for glm because it's a header only library and because we shipped it with the project
# normally you would use find_package(<package_name>) for libraries with actual objects
set(GLM_INCLUDE_DIRS libs/glm-0.9.7.2)

find_package(SDL2 REQUIRED)

include_directories($src)
include_directories(SYSTEM libs/sdw)
include_directories(SYSTEM libs)
include_directories(SYSTEM ${SDL2_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})

add_executable(Renderer
        libs/sdw/CanvasPoint.cpp
        libs/sdw/CanvasTriangle.cpp
        libs/sdw/Colour.cpp
        libs/sdw/DrawingWindow.cpp
        libs/sdw/ModelTriangle.cpp
        libs/sdw/RayTriangleIntersection.cpp
        libs/sdw/TextureMap.cpp
        libs/sdw/Utils.cpp
        ${ALL_SOURCES}
        ${ALL_HEADERS})

if (MSVC)
    target_compile_options(${PROJECT_NAME}
            PUBLIC
            /W3
            /Zc:wchar_t
            )
    set(DEBUG_OPTIONS /MTd)
    set(RELEASE_OPTIONS /MT /GF /Gy /O2 /fp:fast)
    if (NOT DEFINED SDL2_LIBRARIES)
        set(SDL2_LIBRARIES SDL2::SDL2 SDL2::SDL2main)
    endif()
else ()
    target_compile_options(${PROJECT_NAME}
        PUBLIC
        -Wall
        -Wextra
        -Wcast-align
        -Wfatal-errors
        -Werror=return-type
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-ignored-attributes)

    set(DEBUG_OPTIONS -O2 -fno-omit-frame-pointer -g)
    set(RELEASE_OPTIONS -O3 -march=native -mtune=native)
    target_link_libraries(${PROJECT_NAME} PUBLIC $<$<CONFIG:Debug>:-Wl,-lasan>)

endif()


target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RelWithDebInfo>:${RELEASE_OPTIONS}>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:Release>:${RELEASE_OPTIONS}>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:Debug>:${DEBUG_OPTIONS}>")
 
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
