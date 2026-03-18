cmake_minimum_required(VERSION 3.16)

# Include FetchContent to download Catch2 automatically
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a newer release
)
FetchContent_MakeAvailable(Catch2)

add_executable(test_dsp
    test_eq.cpp
    ../src/dsp/dsp_engine.cpp
    ../src/dsp/cabsim.cpp
)

target_include_directories(test_dsp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${FFTW3F_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/NeuralAmpModelerCore/NAM
)

target_link_libraries(test_dsp PRIVATE
    Catch2::Catch2WithMain
    ${FFTW3F_LIBRARIES}
    NAM
    sndfile
    Threads::Threads
)

# Register test
include(Catch)
catch_discover_tests(test_dsp)
