cmake_minimum_required(VERSION 3.28)

set(TESTS_WORKDIR "/var/tmp/libbsarchpp" CACHE PATH "Work directory")
set(TESTS_DATADIR "/opt/archives" CACHE PATH "Archive base directory")

# Multithreading
option(TESTS_MULTITHREADED_EXTRACTION "Enable multithreaded extraction" ON)
option(TESTS_MULTITHREADED_PACKING "Enable multithreaded packing" OFF)

# Morrowind
option(TESTS_TES3_ENABLE "Enable Morrowind tests" OFF)
set(TESTS_TES3_DATADIR CACHE PATH "Morrowind data directory")

# Oblivion
option(TESTS_TES4_ENABLE "Enable Oblivion tests" OFF)
set(TESTS_TES4_DATADIR CACHE PATH "Oblivion data directory")

# Skyrim
option(TESTS_TES5_ENABLE "Enable Skyrim tests" OFF)
set(TESTS_TES5_DATADIR CACHE PATH "Skyrim data directory")

# Skyrim SE
option(TESTS_SSE_ENABLE "Enable Skyrim SE tests" OFF)
set(TESTS_SSE_DATADIR CACHE PATH "Skyrim SE data directory")

# Skyrim VR
option(TESTS_TES5VR_ENABLE "Enable Skyrim VR tests" OFF)
set(TESTS_TES5VR_DATADIR CACHE PATH "Skyrim VR data directory")

# Fallout 3
option(TESTS_FO3_ENABLE "Enable Fallout 3 tests" OFF)
set(TESTS_FO3_DATADIR CACHE PATH "Fallout 3 data directory")

# Fallout: New Vegas
option(TESTS_FONV_ENABLE "Enable Fallout: New Vegas tests" OFF)
set(TESTS_FONV_DATADIR CACHE PATH "Fallout: New Vegas data directory")

# Fallout 4
option(TESTS_FO4_ENABLE "Enable Fallout 4 tests" OFF)
set(TESTS_FO4_DATADIR CACHE PATH "Fallout 4 data directory")

# Fallout 4 VR
option(TESTS_FO4VR_ENABLE "Enable Fallout 4 VR tests" OFF)
set(TESTS_FO4VR_DATADIR CACHE PATH "Fallout 4 VR data directory")

# Starfield
option(TESTS_SF_ENABLE "Enable Starfield tests" OFF)
set(TESTS_SF_DATADIR CACHE PATH "Starfield data directory")

find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(libbsarchpp_tests)

set(SOURCES BsaTest.cpp tests_bsa.cpp tests_hash.cpp tests_utils.cpp tests_misc.cpp tests_stringencoding.cpp $<TARGET_OBJECTS:libbsarchpp>)
set(HEADERS BsaTest.h)

if(TESTS_TES3_ENABLE)
    set(SOURCES ${SOURCES} tests_tes3.cpp)
endif()
if(TESTS_TES4_ENABLE)
    set(SOURCES ${SOURCES} tests_tes4.cpp)
endif()
if(TESTS_TES5_ENABLE)
    set(SOURCES ${SOURCES} tests_tes5.cpp)
endif()
if(TESTS_TES5VR_ENABLE)
    set(SOURCES ${SOURCES} tests_tes5vr.cpp)
endif()
if(TESTS_SSE_ENABLE)
    set(SOURCES ${SOURCES} tests_sse.cpp)
endif()
if(TESTS_FO3_ENABLE)
    set(SOURCES ${SOURCES} tests_fo3.cpp)
endif()
if(TESTS_FONV_ENABLE)
    set(SOURCES ${SOURCES} tests_fonv.cpp)
endif()
if(TESTS_FO4_ENABLE)
    set(SOURCES ${SOURCES} tests_fo4.cpp)
endif()
if(TESTS_FO4VR_ENABLE)
    set(SOURCES ${SOURCES} tests_fo4vr.cpp)
endif()
if(TESTS_SF_ENABLE)
    set(SOURCES ${SOURCES} tests_sf.cpp)
endif()

target_sources(libbsarchpp_tests PRIVATE ${SOURCES} ${HEADERS})

# Copy properties from mylib to ensure compatible object files
set(PROPERTIES_TO_COPY
        COMPILE_DEFINITIONS
        INCLUDE_DIRECTORIES
)
foreach(PROPERTY IN LISTS PROPERTIES_TO_COPY)
    set_property(TARGET libbsarchpp_tests APPEND PROPERTY
            ${PROPERTY} $<TARGET_PROPERTY:libbsarchpp,${PROPERTY}>
    )
endforeach()

configure_file(settings.h.in settings.h @ONLY)
target_include_directories(libbsarchpp_tests PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

set_target_properties(libbsarchpp_tests PROPERTIES CXX_STANDARD 23)
if (MSVC)
    target_compile_options(libbsarchpp_tests PRIVATE /W4)
else()
    target_compile_options(libbsarchpp_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(
        libbsarchpp_tests
        PRIVATE
        GTest::gtest_main
        OpenSSL::SSL
        libbsarchpp
)
if (WIN32)
    add_custom_command(TARGET libbsarchpp_tests POST_BUILD COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:libbsarchpp>" "$<CONFIGURATION>/$<TARGET_FILE_NAME:libbsarchpp>" COMMENT "Copy dll to test exe path")
endif ()

add_custom_command(TARGET libbsarchpp_tests POST_BUILD COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/tests/files" "files" COMMENT "Copy test files")

include(GoogleTest)
gtest_discover_tests(libbsarchpp_tests)
