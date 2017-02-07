cmake_minimum_required(VERSION 3.0)

message("CMAKE_JS_INC:" ${CMAKE_JS_INC})

set (CMAKE_CXX_STANDARD 11)
set (PROJECT_NAME vstjs)
set (BIN_NAME vstjs-bin)

# Name of the project (will be the name of the plugin)
project(vstjs)

# If VST3 sdk path is provided via env variable, use that
if(NOT DEFINED $ENV{VST3_SDK_PATH})
    set(VST3_SDK_PATH "~/SDKS/VST3 SDK")
else()
    set(VST3_SDK_PATH $ENV{VST3_SDK_PATH})
endif()

# Grab boost dependency via boost-lib npm module
SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/node_modules/boost-lib/cmake")
include(BoostLib)
require_boost_libs(">= 1.6.0" system)

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

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_JS_INC}
    ${Boost_INCLUDE_DIR}
)
target_include_directories(${BIN_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include/JUCE/modules
    ${VST3_SDK_PATH}
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
    ${Boost_LIBRARIES}
)

target_link_libraries(
        ${BIN_NAME}
        ${JUCE_PLATFORM_SPECIFIC_LIBRARIES}
)