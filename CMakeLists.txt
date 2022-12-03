cmake_minimum_required(VERSION 3.22)
project(inja_cli
    VERSION 0.0.0
    DESCRIPTION "Inja CLI"
    HOMEPAGE_URL "https://github.com/laurence-myers/inja-cli"
    LANGUAGES CXX
    )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POLICY_DEFAULT_CMP0135 NEW) # Avoid a CMake warning

################
# Platform config

# Fix DLL not found problem on Windows
# Note: Windows shared libraries do not have RPATH or RUNPATH
# DLLs are searched first at current directories and then in directories
# listed in $PATH variable.
# @see https://caiorss.github.io/C-Cpp-Notes/building-system-cmake.html#orgfdf5850
if (WIN32)
    set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
endif ()

# Statically link the Windows runtime dependencies
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY  "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

###################
# Install CPM.cmake

set(CPM_DOWNLOAD_VERSION 0.35.0)

if (CPM_SOURCE_CACHE)
    set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif (DEFINED ENV{CPM_SOURCE_CACHE})
    set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else ()
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif ()

if (NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CPM_DOWNLOAD_LOCATION}
        )
endif ()

include(${CPM_DOWNLOAD_LOCATION})

##############
# Dependencies

CPMAddPackage(
    NAME nowide
    VERSION 11.1.4
    URL "https://github.com/boostorg/nowide/releases/download/v11.1.4/nowide_standalone_v11.1.4.tar.gz"
)

CPMAddPackage("gh:pantor/inja@3.3.0")

CPMAddPackage(
    NAME tclap
    VERSION 1.4.0-rc1
    URL "https://ixpeering.dl.sourceforge.net/project/tclap/tclap-1.4.0-rc1.tar.bz2"
)
if(tclap_ADDED)
    add_library(tclap INTERFACE IMPORTED)
    target_include_directories(
        tclap SYSTEM INTERFACE ${tclap_SOURCE_DIR}/include
    )
endif()

##############
# App settings

add_executable(inja_cli main.cpp)

# Set the output exe name
set_target_properties(inja_cli PROPERTIES OUTPUT_NAME "inja")

###################
# Link dependencies

target_link_libraries(inja_cli nowide inja tclap)

################
# Package as zip

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
install(
    FILES
    ${CPACK_RESOURCE_FILE_LICENSE}
    ${CPACK_RESOURCE_FILE_README}
    DESTINATION
    .
)

include(CPackComponent)
cpack_add_component(
    inja_cli
    DISPLAY_NAME "Inja CLI"
    REQUIRED
)

install(
    TARGETS
    inja_cli
    RUNTIME
    DESTINATION
    .
    COMPONENT
    inja_cli
)

set(CPACK_BINARY_NSIS "OFF")
set(CPACK_BINARY_ZIP "ON")
set(CPACK_PACKAGE_NAME "inja_cli")
set(CPACK_PACKAGE_VENDOR "Laurence Dougal Myers")

include(CPack)
