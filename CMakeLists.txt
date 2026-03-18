cmake_minimum_required(VERSION 3.16)
project(rockstar VERSION 0.1.0 LANGUAGES C CXX)

# Standard C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find Packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtkmm-3.0)
pkg_check_modules(JACK REQUIRED jack)
pkg_check_modules(FFTW3F REQUIRED fftw3f)
find_package(SndFile REQUIRED) # For IR Loading (sndfile)
find_package(Threads REQUIRED)

# Add NAM Core as a subdirectory
add_subdirectory(vendor/NeuralAmpModelerCore)

# Executable setup
add_executable(rockstar
    src/main.cpp
    src/host/jack_host.cpp
    src/dsp/dsp_engine.cpp
    src/dsp/cabsim.cpp
)

# Include directories
target_include_directories(rockstar PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${GTK3_INCLUDE_DIRS}
    ${JACK_INCLUDE_DIRS}
    ${FFTW3F_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/NeuralAmpModelerCore/NAM
)

# Link libraries
target_link_directories(rockstar PRIVATE
    ${GTK3_LIBRARY_DIRS}
    ${JACK_LIBRARY_DIRS}
    ${FFTW3F_LIBRARY_DIRS}
)

target_link_libraries(rockstar PRIVATE
    ${GTK3_LIBRARIES}
    ${JACK_LIBRARIES}
    ${FFTW3F_LIBRARIES}
    NAM
    sndfile
    Threads::Threads
)

# Installation
install(TARGETS rockstar
    RUNTIME DESTINATION bin
)

# Testing (Catch2 config later linked)
enable_testing()
add_subdirectory(tests)

# Packaging (CPack -> .deb file)
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "rockstar")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Real-Time standalone Linux audio application with NAM, CabSim, and EQ")
set(CPACK_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Gabriel Benselum")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtk-3-0, libgtkmm-3.0-1v5, libjack-jackd2-0, libfftw3-single3, libsndfile1")
include(CPack)
