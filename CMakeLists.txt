cmake_minimum_required(VERSION 3.28)

include(CMakePackageConfigHelpers)

project(libbsarchpp CXX)
set(CMAKE_CXX_STANDARD 23)

# USE_VCPKG gets set by CMakePresets
if (USE_VCPKG)
    find_package(OpenSSL REQUIRED)
    find_package(lz4 CONFIG REQUIRED)
    find_package(ZLIB REQUIRED)
endif ()

add_library(libbsarchpp SHARED)

target_sources(libbsarchpp
        PRIVATE
        src/Bsa.cpp
        src/types.cpp
        src/hash.cpp
        src/utils/utils.cpp
        PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
        FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libbsarchpp/Bsa.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libbsarchpp/enums.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libbsarchpp/constants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libbsarchpp/dllexport.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libbsarchpp/types.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libbsarchpp/hash.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libbsarchpp/utils/utils.h
)
target_include_directories(libbsarchpp PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libbsarchpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libbsarchpp/utils)

if (USE_VCPKG)
    target_link_libraries(libbsarchpp PRIVATE OpenSSL::SSL lz4::lz4 ZLIB::ZLIB)
else ()
    target_link_libraries(libbsarchpp PRIVATE ssl lz4 z)
endif ()
if (NOT MSVC)
    # tbb is required for parallelization when using gcc or clang
    target_link_libraries(libbsarchpp PRIVATE tbb)
endif ()

set_target_properties(libbsarchpp PROPERTIES
        DEBUG_POSTFIX "d"
        OUTPUT_NAME "bsarchpp"
)

# compile definitions
# gsl::narrow calls abort() if this is not set
target_compile_definitions(libbsarchpp PUBLIC gsl_CONFIG_NARROW_THROWS_ON_TRUNCATION=1)
if (MSVC)
    # disable crt deprecation warnings
    target_compile_definitions(libbsarchpp PRIVATE _CRT_SECURE_NO_DEPRECATE)
endif ()

# compile options
if (MSVC)
    target_compile_options(libbsarchpp PRIVATE /MP /W4)
else ()
    target_compile_options(libbsarchpp PRIVATE -Wall -Wextra -Wpedantic -fvisibility=hidden)
endif ()

# install
install(TARGETS libbsarchpp EXPORT libbsarchppTargets FILE_SET HEADERS)
if (WIN32)
    install(FILES $<TARGET_PDB_FILE:libbsarchpp> DESTINATION pdb OPTIONAL)
endif ()
install(EXPORT libbsarchppTargets
        FILE libbsarchpp-targets.cmake
        DESTINATION lib/cmake/libbsarchpp
)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/libbsarchpp-config.cmake"
        INSTALL_DESTINATION "lib/cmake/libbsarchpp"
        NO_SET_AND_CHECK_MACRO
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/libbsarchpp-config-version.cmake"
        VERSION "1.0.0"
        COMPATIBILITY AnyNewerVersion
        ARCH_INDEPENDENT
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/libbsarchpp-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/libbsarchpp-config-version.cmake
        DESTINATION lib/cmake/libbsarchpp
)

# tests
option(LIBBSARCHPP_TESTING "Build tests" OFF)
if (LIBBSARCHPP_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif ()