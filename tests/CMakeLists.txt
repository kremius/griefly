CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
 
PROJECT(googletest-download NONE)

INCLUDE(ExternalProject)
ExternalProject_Add(googletest
    GIT_REPOSITORY    https://github.com/google/googletest.git
    GIT_TAG           d225acc90bc3a8c420a9bcd1f033033c1ccd7fe0
    SOURCE_DIR        "${CMAKE_BINARY_DIR}/googletest-src"
    BINARY_DIR        "${CMAKE_BINARY_DIR}/googletest-build"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ""
    INSTALL_COMMAND   ""
    TEST_COMMAND      ""
)
