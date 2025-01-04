cmake_minimum_required(VERSION 3.28)

set(TESTS_WORKDIR "/var/tmp/libbsarchpp" CACHE PATH "Work directory")
set(TESTS_DATADIR "/opt/archives" CACHE PATH "Archive base directory")

# Multithreading
option(TESTS_MULTITHREADED_EXTRACTION "Enable multithreaded extraction" ON)
option(TESTS_MULTITHREADED_PACKING "Enable multithreaded packing" OFF)

# Morrowind
option(TESTS_TES3_ENABLE "Enable Morrowind tests" ON)
set(TESTS_TES3_DATADIR CACHE PATH "Morrowind data directory")

# Oblivion
option(TESTS_TES4_ENABLE "Enable Oblivion tests" ON)
set(TESTS_TES4_DATADIR CACHE PATH "Oblivion data directory")

# Skyrim
option(TESTS_TES5_ENABLE "Enable Skyrim tests" ON)
set(TESTS_TES5_DATADIR CACHE PATH "Skyrim data directory")

# Skyrim SE
option(TESTS_SSE_ENABLE "Enable Skyrim SE tests" ON)
set(TESTS_SSE_DATADIR CACHE PATH "Skyrim SE data directory")

# Skyrim VR
option(TESTS_SKYRIMVR_ENABLE "Enable Skyrim VR tests" ON)
set(TESTS_SKYRIMVR_DATADIR CACHE PATH "Skyrim VR data directory")

# Fallout 3
option(TESTS_FO3_ENABLE "Enable Fallout 3 tests" ON)
set(TESTS_FO3_DATADIR CACHE PATH "Fallout 3 data directory")

# Fallout: New Vegas
option(TESTS_FONV_ENABLE "Enable Fallout: New Vegas tests" ON)
set(TESTS_FONV_DATADIR CACHE PATH "Fallout: New Vegas data directory")

# Fallout 4
option(TESTS_FO4_ENABLE "Enable Fallout 4 tests" ON)
set(TESTS_FO4_DATADIR CACHE PATH "Fallout 4 data directory")

# Fallout 4 VR
option(TESTS_FO4VR_ENABLE "Enable Fallout 4 VR tests" ON)
set(TESTS_FO4VR_DATADIR CACHE PATH "Fallout 4 VR data directory")

# Starfield
option(TESTS_SF_ENABLE "Enable Starfield tests" ON)
set(TESTS_SF_DATADIR CACHE PATH "Starfield data directory")

find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)

enable_testing()
add_executable(
        tests
        tests.cpp
)

configure_file(settings.h.in settings.h @ONLY)
target_include_directories(tests PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

set_target_properties(tests PROPERTIES CXX_STANDARD 23)
target_link_libraries(
        tests
        PRIVATE
        GTest::gtest_main
        OpenSSL::SSL
        libbsarchpp
)
if (WIN32)
    add_custom_command(TARGET tests POST_BUILD COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:libbsarchpp>" "$<CONFIGURATION>/$<TARGET_FILE_NAME:libbsarchpp>" COMMENT "Copy dll to test exe path")
endif ()

add_custom_command(TARGET tests POST_BUILD COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/tests/files" "files" COMMENT "Copy test files")

include(GoogleTest)
gtest_discover_tests(tests)