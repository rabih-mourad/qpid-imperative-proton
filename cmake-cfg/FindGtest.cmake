# The following variables are set if gtest is found.
#   GTEST_FOUND             - Set to false or undefined if gtest isn't found.
#   GTEST_INCLUDE_DIR       - The gtest include directory.
#   GTEST_BOTH_LIBRARIES    - The gtest library to link against.

include(FindPackageHandleStandardArgs)
include(SelectLibraryConfigurations)

IF(NOT GTEST_FOUND)
    SET(GTEST_ROOT "${CMAKE_SOURCE_DIR}/thirdparty/gtest" CACHE PATH "Installation prefix for gtest")

    SET(_DIST_PATH ${GTEST_ROOT})

    FIND_PATH(
        GTEST_INCLUDE_DIR
        gtest/gtest.h
        HINTS ${GTEST_ROOT}/include
        NO_DEFAULT_PATH
    )
    FIND_LIBRARY(
        GTEST_LIBRARY_RELEASE gtest
        PATH ${_DIST_PATH}/release/lib
        NO_DEFAULT_PATH
    )
    FIND_LIBRARY(
        GTEST_LIBRARY_DEBUG gtest
        PATH ${_DIST_PATH}/debug/lib
        NO_DEFAULT_PATH
    )
    SELECT_LIBRARY_CONFIGURATIONS(GTEST)

	FIND_LIBRARY(
        GTEST_MAIN_LIBRARY_RELEASE gtest_main
        PATH ${_DIST_PATH}/release/lib
        NO_DEFAULT_PATH
    )
    FIND_LIBRARY(
        GTEST_MAIN_LIBRARY_DEBUG gtest_main
        PATH ${_DIST_PATH}/debug/lib
        NO_DEFAULT_PATH
    )
    SELECT_LIBRARY_CONFIGURATIONS(GTEST_MAIN)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(
        GTEST DEFAULT_MSG
        GTEST_LIBRARIES
        GTEST_MAIN_LIBRARIES
        GTEST_INCLUDE_DIR
    )
	if(GTEST_FOUND)
        set(GTEST_BOTH_LIBRARIES ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
    endif()

    UNSET(_DIST_PATH)
ENDIF()
