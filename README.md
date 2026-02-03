# libbsarchpp

This is a reimplementation of
xEdit's [BSArchive](https://github.com/TES5Edit/TES5Edit/blob/1fcec21b354786fd6e023d1d38360770557e5a74/Core/wbBSArchive.pas)
in C++ with a focus on cross-platform compatibility. It has been tested
on Linux and Windows.
For the most part, I just copied the Delphi code, converted it to C++, and then made some modifications.

## Limitations / differences compared to xEdit

- When extracting an archive, the path casing can be different compared to using xEdit multithreaded extraction, if the
  paths inside the archive are inconsistent.
    - FO4 VR: extracting "Fallout4 - Misc - Debug.ba2" will create "scripts/Hardcore" directory, using xEdit may
      create "scripts/hardcore" instead.
- Multithreaded packing produces indeterministic results, as the file order in the data section may change, which causes
  the unit tests to fail but improves performance. xEdit also exhibits this behaviour.
- LZ4 compressed archives (SSE, SkyrimVR, SFdds) differ from ones created with xEdit (it apparently uses lz4 r127,
  which is almost a decade old).
- xEdit can create bsa archives > 4GiB, while libbsarchpp will throw an exception because the file offset (uint32_t)
  would overflow.

## Dependencies

- OpenSSL 3 (tested with 3.3.2)
- lz4 (tested with 1.10)
- zlib (tested with 1.3.1)
- GTest if building tests (tested with 1.14.0 and 1.15.2)
- C++23 compatible compiler (e.g. GCC 14, clang 18, Visual Studio 2022)

## Building

[vcpkg](https://github.com/microsoft/vcpkg) can be used for automatic dependency configuration.

See examples below.

### Linux

Install missing dependencies with your package manager. Exact package names are dependent on your distro.

```shell
# create and enter build directory
mkdir build && cd build

# configure, append -DLIBBSARCHPP_TESTING=ON to enable tests
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo

# optionally configure tests using ccmake or cmake-gui if they were included in previous step
ccmake .

# compile with $(nproc) jobs
make -j$(nproc)
```

#### With vcpkg

```shell
# set to the appropriate path for VCPKG
export VCPKG_ROOT=/usr/share/vcpkg

# configure, append -DLIBBSARCHPP_TESTING=ON to enable tests
cmake --preset linux

# optionally configure tests using ccmake or cmake-gui if they were included in previous step
ccmake build

# build libbsarchpp
cmake --build build --config RelWithDebInfo

# run tests
ctest --test-dir build -C RelWithDebInfo --output-on-failure
```

### Windows

```pwsh
# set to the appropriate path for VCPKG
$env:VCPKG_ROOT = "C:\vcpkg"

cmake --preset vs2022-windows -DCMAKE_INSTALL_PREFIX=install 

# optionally configure tests here using ccmake or cmake-gui
ccmake build
    
# build libbsarchpp
cmake --build build --config RelWithDebInfo

# install libbsarchpp
cmake --install build --config RelWithDebInfo

# run tests
ctest --test-dir build -C RelWithDebInfo --output-on-failure
```

## Usage

### CMake

Clone the repository and add

```cmake
add_subdirectory(<libbsarchpp directory>)
target_link_libraries(<target> PRIVATE libbsarchpp)
```

or add

```cmake
include(FetchContent)
FetchContent_Declare(
        libbsarchpp
        GIT_REPOSITORY https://github.com/Kaedras/libbsarchpp.git
)
FetchContent_MakeAvailable(libbsarchpp)
target_link_libraries(<target> PRIVATE libbsarchpp)
```

to your CMakeLists.txt.

### Vcpkg

TODO: add this section

### Examples

For simple extraction or creation of archive files you can use the static functions below after including
``libbsarchpp/Bsa.h``.

```c++
static void libbsarchpp::Bsa::extract(const std::filesystem::path &archivePath,
                        const std::filesystem::path &outputDirectory,
                        bool multithreaded = false);

static void libbsarchpp::Bsa::create(const std::filesystem::path &archivePath,
                   ArchiveType type,
                   const std::filesystem::path &inputDirectory,
                   BsaCreationSettings settings);
```

#### Extraction

```c++
Bsa::extract("path/to/archive.bsa", "outputPath");
```

#### Creating a new archive

```c++
Bsa::create("path/to/archive.bsa", ArchiveType::SSE, "inputPath");
Bsa::create("path/to/archive.bsa", ArchiveType::SSE, "inputPath", {.compressed = true, .multithreaded = true});
```

#### Advanced usage

For advanced usage you can start by taking a look at the static functions detailed above.

Additional documentation can be found in Bsa.h and can also be generated
using [Doxygen](https://github.com/doxygen/doxygen).

## Testing

The unit tests unpack and repack the bsa/ba2 files of most Bethesda games and validate the checksums.
Run ``ccmake build`` or ``cmake-gui build`` after the initial cmake configuration and set at least TESTS_DATADIR and
TESTS_WORKDIR. You can enable/disable individual games and set the data paths for each game.
If no game-specific directories are provided, they default to ``TESTS_DATADIR/<game name>`` e.g. ``/opt/archives/tes3``

The checksums were created by unpacking game files and repacking them with xEdit.
The scripts used can be found in tests/utils.

### Notes:

- The tests will fail if using multithreaded packing because of a limitation mentioned above.
- Linux specific:
    - You may get ENOSPC (No space left on device) if you leave the work directory at the default value and your distro
      uses tmpfs for temporary files.
    - To improve performance, you should consider setting the work directory to tmpfs for improved performance (10GiB
      recommended if running tests for Skyrim and later titles, 20GiB for Starfield) by running
      ``sudo mount -t tmpfs -o size=20g tmpfs <path>``. There does not seem to be a tmpfs equivalent for Windows.

## Credits

The original Delphi code can be
found [here](https://github.com/TES5Edit/TES5Edit/blob/1fcec21b354786fd6e023d1d38360770557e5a74/Core/wbBSArchive.pas).

CMakePresets.json and parts of the README from [here](https://github.com/ModOrganizer2/modorganizer/tree/dev/vcpkg) were
used as a base.

.clang-format was copied from [here](https://github.com/ModOrganizer2/libbsarch/blob/master/.clang-format).

gsl-lite can be found [here](https://github.com/gsl-lite/gsl-lite).

dxgiformat.h can be found [here](https://github.com/microsoft/DirectX-Headers/blob/main/include/directx/dxgiformat.h).