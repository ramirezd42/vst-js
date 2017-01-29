cmake_minimum_required(VERSION 3.0)

message("CMAKE_JS_INC:" ${CMAKE_JS_INC})

set (CMAKE_CXX_STANDARD 11)
set (BIN_NAME vstjs-bin)

# Name of the project (will be the name of the plugin)
project(vstjs)

link_directories()

# Build a shared library named after the project from the files in `src/`
file(GLOB SHARED_SOURCE_FILES "shared/proto/build/*.pb.cc" "shared/JuceLibraryCode/*.mm")
file(GLOB BIN_SOURCE_FILES "src/host/*.cpp" "src/host/*.h")
file(GLOB MODULE_SOURCE_FILES "src/module/*.cpp" "src/module/*.h")
add_library(${PROJECT_NAME} SHARED ${SHARED_SOURCE_FILES} ${MODULE_SOURCE_FILES})
add_executable(${BIN_NAME} ${SHARED_SOURCE_FILES} ${BIN_SOURCE_FILES})

# Gives our library file a .node extension without any "lib" prefix
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# Essential include files to build a node addon,
# You should add this line in every CMake.js based project
include_directories(
    ${CMAKE_SOURCE_DIR}/shared/proto/build
    ${CMAKE_SOURCE_DIR}/JuceLibraryCode
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/JUCE/modules
    ${CMAKE_SOURCE_DIR}/include/VST3
    ${CMAKE_SOURCE_DIR}/include/zmq
    /Users/dxr224/Projects/grpc/include

)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC})
target_include_directories(${BIN_NAME} PRIVATE ${CMAKE_JS_INC})

# Essential library files to link to a node addon
# You should add this line in every CMake.js based project
# osx platform specific libraries
SET(JUCE_PLATFORM_SPECIFIC_LIBRARIES
    "-framework Accelerate"
    "-framework AudioToolbox"
    "-framework AudioUnit"
    "-framework Carbon"
    "-framework Cocoa"
    "-framework CoreAudio"
    "-framework CoreAudioKit"
    "-framework CoreMidi"
    "-framework DiscRecording"
    "-framework IOKit"
    "-framework OpenGL"
    "-framework QTKit"
    "-framework QuartzCore"
    "-framework QuickTime"
    "-framework WebKit"
    "-lobjc")

target_link_libraries(
    ${PROJECT_NAME}
    ${CMAKE_JS_LIB}
    ${JUCE_PLATFORM_SPECIFIC_LIBRARIES}
    ${CMAKE_SOURCE_DIR}/lib/libprotobuf.a
    ${CMAKE_SOURCE_DIR}/lib/libzmq.a
)

target_link_libraries(
        ${BIN_NAME}
        ${CMAKE_JS_LIB}
        ${JUCE_PLATFORM_SPECIFIC_LIBRARIES}
        ${CMAKE_SOURCE_DIR}/lib/libprotobuf.a
        ${CMAKE_SOURCE_DIR}/lib/libzmq.a
)