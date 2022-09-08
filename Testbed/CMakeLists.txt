#-------------------------------------------------------
# Testbed Main Build Script
#-------------------------------------------------------
cmake_minimum_required(VERSION 3.0)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_BINARY_DIR})

project(testbed)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 14)

if (USE_CONAN)
    if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
        message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
        file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
    endif ()

    include(${CMAKE_BINARY_DIR}/conan.cmake)
    conan_cmake_configure(REQUIRES angelscript/2.35.1 catch2/3.1.0 GENERATORS cmake_find_package)
    conan_cmake_autodetect(settings)
    conan_cmake_install(PATH_OR_REFERENCE . BUILD missing SETTINGS ${settings})
endif ()

find_package(Angelscript REQUIRED)
find_package(Catch2)


file(COPY "${CMAKE_SOURCE_DIR}/../Example.as" DESTINATION "${CMAKE_BINARY_DIR}")

set(SRC
        ../RefCountingObject.h
        ../RefCountingObjectPtr.h
        debug_log.h
        scriptstdstring.h

        ../Example.cpp

        scriptstdstring.cpp
        )

add_executable(${PROJECT_NAME} ${SRC} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE Angelscript::angelscript)

if (Catch2_FOUND)
    include(CTest)
    include(Catch)
    enable_testing()

    set(TEST_PR_NAME "${PROJECT_NAME}_tests")

    add_executable(${TEST_PR_NAME} ${SRC} test.cpp)
    target_link_libraries(${TEST_PR_NAME} PRIVATE Catch2::Catch2 Angelscript::angelscript Catch2::Catch2WithMain)

    catch_discover_tests(${TEST_PR_NAME})
endif ()

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE Winmm)
    if (Catch2_FOUND)
        target_link_libraries(${TEST_PR_NAME} PRIVATE Winmm)
    endif ()
endif ()