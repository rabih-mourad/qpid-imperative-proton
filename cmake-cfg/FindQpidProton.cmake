# The following variables are set if qpid_proton is found.
#   QPID_PROTON_FOUND             - Set to false or undefined if Qpid Proton isn't found.
#   QPID_PROTON_INCLUDE_DIR       - The Qpid Proton include directory.
#   QPID_PROTON_CPP_LIBRARIES     - Addition library to link against for C++ bindings
#   QPID_PROTON_RUNTIME_LIBRARIES - The path of the runtime libraries

include(FindPackageHandleStandardArgs)
include(SelectLibraryConfigurations)

IF(NOT QPID_PROTON_FOUND)

    SET(QPID_PROTON_ROOT "${CMAKE_SOURCE_DIR}/thirdparty/qpid-proton" CACHE PATH "Installation prefix for Qpid Proton")

    SET(_DIST_PATH ${QPID_PROTON_ROOT})

    FIND_PATH(
        QPID_PROTON_INCLUDE_DIR 
        proton/version.h
        HINTS ${_DIST_PATH}/include
        NO_DEFAULT_PATH
    )

    FIND_LIBRARY(
        QPID_PROTON_CORE_LIBRARY_RELEASE qpid-proton-core
        HINTS ${_DIST_PATH}/release/bin
        NO_DEFAULT_PATH
    )
    FIND_LIBRARY(
        QPID_PROTON_CORE_LIBRARY_DEBUG qpid-proton-core
        HINTS ${_DIST_PATH}/debug/bin
        NO_DEFAULT_PATH
    )
    SELECT_LIBRARY_CONFIGURATIONS(QPID_PROTON_CORE)

    FIND_LIBRARY(
        QPID_PROTON_PROACTOR_LIBRARY_RELEASE qpid-proton-proactor
        HINTS ${_DIST_PATH}/release/bin
        NO_DEFAULT_PATH
    )
    FIND_LIBRARY(
        QPID_PROTON_PROACTOR_LIBRARY_DEBUG qpid-proton-proactor
        HINTS ${_DIST_PATH}/debug/bin
        NO_DEFAULT_PATH
    )
    SELECT_LIBRARY_CONFIGURATIONS(QPID_PROTON_PROACTOR)

    FIND_LIBRARY(
        QPID_PROTON_CPP_LIBRARY_RELEASE qpid-proton-cpp
        HINTS ${_DIST_PATH}/release/bin
        NO_DEFAULT_PATH
    )
    FIND_LIBRARY(
        QPID_PROTON_CPP_LIBRARY_DEBUG qpid-proton-cpp
        HINTS ${_DIST_PATH}/debug/bin
        NO_DEFAULT_PATH
    )
    SELECT_LIBRARY_CONFIGURATIONS(QPID_PROTON_CPP)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(
        QpidProton DEFAULT_MSG
        QPID_PROTON_CORE_LIBRARIES
        QPID_PROTON_PROACTOR_LIBRARIES
        QPID_PROTON_CPP_LIBRARIES
        QPID_PROTON_INCLUDE_DIR
    )

    unset(_DIST_PATH)
ENDIF()
