cmake_minimum_required(VERSION 3.0)

message("CMAKE_JS_INC:" ${CMAKE_JS_INC})

set (CMAKE_CXX_STANDARD 11)
set (PROJECT_NAME vstjs)
set (BIN_NAME vstjs-bin)

# Name of the project (will be the name of the plugin)
project(vstjs)

# Build a shared library named after the project from the files in `src/`
file(GLOB BIN_SOURCE_FILES "src/host/*.cpp" "src/host/*.h" "src/host/JuceLibraryCode/*.mm")
file(GLOB MODULE_SOURCE_FILES "src/module/*.cpp" "src/module/*.h")

add_executable(${BIN_NAME} ${BIN_SOURCE_FILES})
add_library(${PROJECT_NAME} SHARED ${MODULE_SOURCE_FILES})

# Gives our library file a .node extension without any "lib" prefix
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
link_directories()

# Essential include files to build a node addon,
# You should add this line in every CMake.js based project
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/shared)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC})
target_include_directories(${BIN_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include/JUCE/modules
    ${CMAKE_SOURCE_DIR}/include/VST3
)

# Essential library files to link to a node addon
# You should add this line in every CMake.js based project
# osx platform specific libraries
SET(JUCE_PLATFORM_SPECIFIC_LIBRARIES
    "-framework Accelerate"
    "-framework AudioToolbox"
    "-framework Cocoa"
    "-framework CoreAudio"
    "-framework CoreAudioKit"
    "-framework CoreMidi"
    "-framework IOKit"
    "-framework OpenGL"
    "-framework QTKit"
    "-framework QuartzCore"
    "-lobjc")

target_link_libraries(
        ${PROJECT_NAME}
        ${CMAKE_JS_LIB}
        /usr/local/Cellar/boost/1.63.0/lib/libboost_system.a
)

target_link_libraries(
        ${BIN_NAME}
        ${JUCE_PLATFORM_SPECIFIC_LIBRARIES}
)