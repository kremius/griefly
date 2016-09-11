cmake_minimum_required(VERSION 2.8)
project(KVEngineQt)

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_COVER "Generate test coverage" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include(ClangFormat)

##################################
### Start search dependencies
###

###########################
#
# Git search
#
###########################
find_package(Git REQUIRED)
if(GIT_FOUND)
    message(STATUS "Git has been found, ${GIT_VERSION_STRING}")
else()
    message(FATAL_ERROR "Unable to find Git!")
endif()

###########################
#
# Qt5 search
#
###########################
find_package(Qt5Core REQUIRED)
if(Qt5Core_FOUND)
    message(STATUS "Qt5Core has been found")
else()
    message(FATAL_ERROR "Unable to find Qt5Core!")
endif()

find_package(Qt5Network REQUIRED)
if(Qt5Network_FOUND)
    message(STATUS "Qt5Network has been found")
else()
    message(FATAL_ERROR "Unable to find Qt5Network!")
endif()

find_package(Qt5Widgets REQUIRED)
if(Qt5Widgets_FOUND)
    message(STATUS "Qt5Widgets has been found")
else()
    message(FATAL_ERROR "Unable to find Qt5Widgets!")
endif()

find_package(Qt5OpenGL REQUIRED)
if(Qt5OpenGL_FOUND)
    message(STATUS "Qt5OpenGL has been found")
else()
    message(FATAL_ERROR "Unable to find Qt5OpenGL!")
endif()

# Some variables neeeded by Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

###########################
#
# Golang search
#
###########################
find_package(Go 1.5 REQUIRED)
if(GO_FOUND)
    message(STATUS "Go has been found, version: ${GO_VERSION}")
else()
    message(FATAL_ERROR "Unable to find Go!")
endif()

###########################
#
# Python 2.7 search
#
###########################
find_package(PythonInterp 2.7 REQUIRED)
if(PYTHONINTERP_FOUND)
    message(STATUS "Python has been found, version: ${PYTHON_VERSION_STRING}")
else()
    message(FATAL_ERROR "Unable to find Python!")
endif()

###########################
#
# SFML search (${SFML_ROOT} and $ENV{SFML_ROOT} can be setted)
#
###########################
find_package(SFML 2 COMPONENTS system audio REQUIRED)
if(SFML_FOUND)
    message(STATUS "SFML has been found, libs: ${SFML_LIBRARIES}")
else()
    message(FATAL_ERROR "Unable to find SFML!")
endif()

###########################
#
# PNG search
#
###########################
if(WIN32)
    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH}
        ${PNG_INCLUDE_DIR}
        $ENV{PNG_INCLUDE_DIR})
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH}
        ${PNG_LIBRARY_DIR}
        $ENV{PNG_LIBRARY_DIR})
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH}
        ${ZLIB_ROOT}
        $ENV{ZLIB_ROOT})
endif()
find_package(PNG REQUIRED)
if(SFML_FOUND)
    message(STATUS "PNG has been found, version: ${PNG_VERSION_STRING}, include: ${PNG_INCLUDE_DIRS}, libs: ${PNG_LIBRARIES}")
else()
    message(FATAL_ERROR "Unable to find PNGLIB!")
endif()

######################
#
# GTEST "search"
#
######################
if(BUILD_TESTS)
    # Props to http://crascit.com/2015/07/25/cmake-gtest/
    # Download and unpack googletest at configure time

    if(MSVC) # VS2012 doesn't support correctly the tuples yet
        add_definitions(/D _VARIADIC_MAX=10)
    endif()

    configure_file(Tests/CMakeLists.txt
                   googletest-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )

    # Prevent GoogleTest from overriding our compiler/linker options
    # when building with Visual Studio
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Add googletest directly to our build. This adds
    # the following targets: gtest, gtest_main, gmock
    # and gmock_main
    add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                     ${CMAKE_BINARY_DIR}/googletest-build
                     EXCLUDE_FROM_ALL)

    if(MSVC)
        add_definitions(/D _BUILD_TESTS)
    else()
        add_definitions(-D _BUILD_TESTS)
    endif()
endif()

###
### End search dependecies
#######################################



#######################################
### Checking build version
###

execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${GIT_EXECUTABLE} rev-parse --git-dir
    OUTPUT_VARIABLE IS_GIT_REPO OUTPUT_STRIP_TRAILING_WHITESPACE)
if(IS_GIT_REPO)
    message(STATUS "Source dir is active git repo")
else()
    message(FATAL_ERROR "Source dir is not active git repo: ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=4
    OUTPUT_VARIABLE CLIENT_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT CLIENT_VERSION)
    # No tags or shallow copy
    set(CLIENT_VERSION "v0.0.0")
endif()
message(STATUS ${CLIENT_VERSION})

# Version.cpp generation
execute_process(
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/version.py ${CLIENT_VERSION}
)

###
### End checking build version
##########################################



###########################################
### Some global compiler flags & other stuff
###

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
endif()

#SET(CMAKE_EXE_LINKER_FLAGS "/DEBUG")

###
### Global compiler glags have been setted
##########################################



##################################
### Building client
###

set(SOURCE_DIR Sources/)
set(EXEC_DIR Exec/)
set(TESTS_DIR Tests/)

include_directories(SOURCE_DIR)
# Workaround for linux
include_directories("Sources")

if(BUILD_TESTS)
    file(GLOB_RECURSE TESTS "${TESTS_DIR}*.cpp" "${TESTS_DIR}*.h")
else()
    set(TESTS "")
endif()

file(GLOB_RECURSE SOURCES "${SOURCE_DIR}*.cpp")
file(GLOB_RECURSE HEADERS "${SOURCE_DIR}*.h")
file(GLOB_RECURSE FORMS "${SOURCE_DIR}*.ui")

# Metadata & source code generation
add_custom_command(
    OUTPUT metadata.json
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/metadata_generator.py ${CMAKE_CURRENT_SOURCE_DIR} metadata.json
    DEPENDS ${HEADERS})
add_custom_command(
    OUTPUT AutogenMetadata.cpp
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/autogen.py
    DEPENDS metadata.json
)
add_custom_target(Metadata ALL DEPENDS AutogenMetadata.cpp)

list(APPEND SOURCES "${CMAKE_BINARY_DIR}/AutogenMetadata.cpp")
list(APPEND SOURCES "${CMAKE_BINARY_DIR}/Version.cpp")

qt5_wrap_ui(FORMS_HEADERS ${FORMS})

add_executable(KVEngine WIN32 ${SOURCES} ${HEADERS} ${FORMS_HEADERS} ${TESTS})

# Add PNG lib
add_definitions(${PNG_DEFINITIONS})
include_directories(${PNG_INCLUDE_DIRS})
target_link_libraries(KVEngine ${PNG_LIBRARIES})

# Add SFML lib
include_directories(${SFML_INCLUDE_DIR})
target_link_libraries(KVEngine ${SFML_LIBRARIES})

# Add Qt5 lib
qt5_use_modules(KVEngine Core)
qt5_use_modules(KVEngine Network)
qt5_use_modules(KVEngine Widgets)
qt5_use_modules(KVEngine OpenGL)

# Add GTEST lib
if(BUILD_TESTS)
    include_directories("${gtest_SOURCE_DIR}/include"
                        "${gmock_SOURCE_DIR}/include")
    target_link_libraries(KVEngine gtest)
    target_link_libraries(KVEngine gmock)
    if(MSVC)
        set_target_properties(KVEngine PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE")
    endif()
endif()

# Add opengl lib
if(WIN32)
    target_link_libraries(KVEngine opengl32)
else()
    target_link_libraries(KVEngine GL)
endif()

# Add coverage support
if(BUILD_COVER)
    if(CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0") # debug, no optimisation
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage") # enabling coverage
        add_definitions(-D _BUILD_COVER)
    else()
        message(FATAL_ERROR "Tests coverage is not supported for Windows!")
    endif()
endif()

###
### End building client
##################################



#####################################
### Building launcher
###

set(LAUNCHER_DIR Launcher/)

include_directories(LAUNCHER_DIR)
# Workaround for linux
include_directories("Launcher")

file(GLOB_RECURSE LAUNCHER_SOURCES "${LAUNCHER_DIR}*.cpp")
file(GLOB_RECURSE LAUNCHER_HEADERS "${LAUNCHER_DIR}*.h")
file(GLOB_RECURSE LAUNCHER_FORMS "${LAUNCHER_DIR}*.ui")

qt5_wrap_ui(LAUNCHER_FORMS_HEADERS ${LAUNCHER_FORMS})

add_executable(Launcher WIN32 ${LAUNCHER_SOURCES} ${LAUNCHER_HEADERS} ${LAUNCHER_FORMS_HEADERS})

# Add Qt5 lib
qt5_use_modules(Launcher Core)
qt5_use_modules(Launcher Widgets)

if(WIN32)
    target_link_libraries(Launcher Qt5::WinMain)
endif()

###
### End building launcher
#####################################



#####################################
### Building qzipunpacker
###

set(UNPACKER_DIR QzipUnpacker/)

include_directories(UNPACKER_DIR)
# Workaround for linux
include_directories("QzipUnpacker")

file(GLOB_RECURSE UNPACKER_SOURCES "${UNPACKER_DIR}*.cpp")

add_executable(Unpacker ${UNPACKER_SOURCES})

# Add Qt5 lib
qt5_use_modules(Unpacker Core)

###
### End building qzipunpacker
#####################################



#####################################
### Building server
###

set(GOPATH ${CMAKE_CURRENT_SOURCE_DIR}/gopath)
set(SERVER_DIR ${GOPATH}/src/griefly-server)

file(GLOB_RECURSE GOFILES "${SERVER_DIR}/*.go")


add_custom_command(
    OUTPUT ${SERVER_DIR}/griefly-server${CMAKE_EXECUTABLE_SUFFIX}
    WORKING_DIRECTORY ${SERVER_DIR}
    COMMAND ${CMAKE_COMMAND} -E env GOPATH=${GOPATH} GO15VENDOREXPERIMENT=1 ${GO_EXECUTABLE} build -ldflags "-X main.clientVersionBuild=${CLIENT_VERSION}"
    DEPENDS ${GOFILES})
add_custom_target(GoServer ALL DEPENDS ${SERVER_DIR}/griefly-server${CMAKE_EXECUTABLE_SUFFIX})

###
### End building server
#####################################



#####################################
### Install rules
###

install(TARGETS KVEngine Launcher
        DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/${EXEC_DIR}/)

install(FILES ${CMAKE_BINARY_DIR}/metadata.json
        DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/${EXEC_DIR}/)

###
### Install rules
#####################################
