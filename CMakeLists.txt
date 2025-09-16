cmake_minimum_required(VERSION 3.28)

include(CMakePackageConfigHelpers)

project(libbsarchpp CXX)
set(CMAKE_CXX_STANDARD 23)

add_library(libbsarchpp SHARED)

# USE_VCPKG gets set by CMakePresets
if (USE_VCPKG)
    find_package(gsl-lite CONFIG REQUIRED)
    find_package(OpenSSL REQUIRED)
    find_package(lz4 CONFIG CONFIG REQUIRED)
    find_package(ZLIB REQUIRED)
    find_package(directx-headers CONFIG REQUIRED)
else()
    # fetch dxgiformat.h
    file(DOWNLOAD
            https://github.com/microsoft/DirectX-Headers/raw/refs/tags/v1.611.0/include/directx/dxgiformat.h
            deps/directx/dxgiformat.h
            EXPECTED_HASH SHA512=004be604e28a60f8f6195144e678a82d10894fc4d1cfea10e7efa72eb7d7cd15efbd03d21ec80a65579c00eec8aa77b09809ade3f1b9198901757ebe7a63401c
    )
    # fetch gsl-lite.hpp
    file(DOWNLOAD
            https://github.com/gsl-lite/gsl-lite/raw/refs/tags/v1.0.1/include/gsl-lite/gsl-lite.hpp
            deps/gsl-lite/gsl-lite.hpp
            EXPECTED_HASH SHA512=54e49559d3b07b1523589428d8c3864a5865b290d8330bae8b29b174469bf56cac41d5e2ed6cfb5fb7cc22adb372f4660938df6dda889c058d5fa39d2f3f7332
    )
    target_include_directories(libbsarchpp PRIVATE ${CMAKE_BINARY_DIR}/deps)
endif ()

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
    target_link_libraries(libbsarchpp PRIVATE
        gsl::gsl-lite gsl::gsl-lite-v1 OpenSSL::SSL OpenSSL::Crypto lz4::lz4 ZLIB::ZLIB Microsoft::DirectX-Headers)
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