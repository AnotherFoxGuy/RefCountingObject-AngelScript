#-------------------------------------------------------
# Testbed Main Build Script
#-------------------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_BINARY_DIR})
cmake_minimum_required(VERSION 3.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 11)

find_package(Angelscript REQUIRED)

project(Testbed)

file(COPY "${CMAKE_SOURCE_DIR}/../Example.as" DESTINATION "${CMAKE_BINARY_DIR}")

set(SRC
    ../RefCountingObject.h
    ../RefCountingObjectHandle.h
    ../RefCountingObjectPtr.h
    debug_log.h
    scriptstdstring.h

    ../Example.cpp
    ../RefCountingObjectHandle.cpp
    main.cpp
    scriptstdstring.cpp
)

add_executable(${PROJECT_NAME} ${SRC})
target_compile_definitions(${PROJECT_NAME} PRIVATE RCO_ENABLE_DEBUGTRACE)
target_link_libraries(${PROJECT_NAME} PRIVATE Angelscript::angelscript)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE Winmm)
endif()