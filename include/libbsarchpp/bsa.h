#pragma once

#include "types.h"  // for FileFO4, FolderTES4, FileTES3

#include <cstdint>     // for uint32_t, uint8_t, int32_t, int64_t, uint64_t
#include <filesystem>  // for path, hash
#include <functional>  // for function
#include <string>      // for operator+, operator""s, string
#include <variant>     // for variant
#include <vector>      // for vector

#ifdef __unix__
#define EXPORT __attribute__((visibility("default")))
#define IMPORT
#else
#define EXPORT _declspec(dllexport)
#define IMPORT _declspec(dllimport)
#endif

#ifdef LIBSARCHPP_BUILD_SHARED
#define DLLEXPORT EXPORT
#elifdef LIBSARCHPP_BUILD_STATIC
#define DLLEXPORT
#else
#define DLLEXPORT IMPORT
#endif

namespace libbsarchpp {

using FileIterationFunction = std::function<bool(const std::filesystem::path&, FilePtr_t, FolderTES4*,
                                                 void*)>;  // filename, file, folder, optional data

/**
 * Class to handle Bethesda archives
 */
class DLLEXPORT Bsa {
public:
  /**
   * @brief Extracts an archive file to a specified output directory.
   * @param archivePath Archive to extract
   * @param outputDirectory Output directory
   * @param multithreaded Enable multithreading
   * @throw std::runtime_error
   */
  static void extract(const std::filesystem::path& archivePath, const std::filesystem::path& outputDirectory,
                      bool multithreaded = false) noexcept(false);

  /**
   * @brief Creates a new archive with the specified type from files inside a specified input directory.
   * @param archivePath Output file path
   * @param type Archive type
   * @param inputDirectory Base directory of input files
   * @param settings Settings to use when creating the archive
   * @throw std::runtime_error
   */
  static void create(const std::filesystem::path& archivePath, ArchiveType type,
                     const std::filesystem::path& inputDirectory,
                     const BsaCreationSettings& settings = {}) noexcept(false);

  /**
   * @brief Reads the header of the specified archive file and returns the archive type.
   * @param archivePath Archive to check
   * @return Type of archive
   * @throw std::runtime_error
   */
  [[nodiscard]] static ArchiveType getArchiveType(const std::filesystem::path& archivePath) noexcept(false);

  /**
   * @brief Opens an existing archive.
   * @param archivePath Archive to open
   * @param multithreaded Enable multithreaded extraction
   * @throw std::runtime_error
   */
  explicit Bsa(const std::filesystem::path& archivePath, bool multithreaded = false) noexcept(false);

  /**
   * @brief Creates a new archive.
   * @param archivePath Archive file to create
   * @param type Archive type
   * @param fileList Predefined list of files
   * @param ddsBasePath Base path for dds files, only required for FO4dds and SFdds archives
   * @param compressed Enable compression
   * @param shareData Identical files will only be written once. Potentially reduces file size while reducing
   * performance
   * @param multithreaded Enable multithreaded packing. Increases performance but produces indeterministic results
   * @throw std::runtime_error
   */
  Bsa(const std::filesystem::path& archivePath, ArchiveType type, std::vector<std::filesystem::path>& fileList,
      std::filesystem::path ddsBasePath, bool compressed, bool shareData, bool multithreaded) noexcept(false);

  /**
   * @copybrief Bsa(const std::filesystem::path&, ArchiveType, std::vector<std::filesystem::path>&, const
   * std::optional<std::filesystem::path>&, bool, bool, bool)
   * @param archivePath Archive file to create
   * @param type Archive type
   * @param fileList Predefined list of files
   * @param ddsBasePath Base path for dds files, only required for FO4dds and SFdds archives
   * @throw std::runtime_error
   */
  Bsa(const std::filesystem::path& archivePath, ArchiveType type, std::vector<std::filesystem::path>& fileList,
      std::filesystem::path ddsBasePath = {}) noexcept(false);

  ~Bsa();

  /**
   * @brief Finalizes a newly created archive.
   * @pre All files have been added
   * @throw std::runtime_error
   */
  void save() noexcept(false);

  /**
   * @brief Adds a file to the archive. It has to be present in the file list provided to the constructor.
   * @param rootDirectory Root directory of files
   * @param filePath File to add
   * @throw std::runtime_error
   */
  void addFile(const std::filesystem::path& rootDirectory, const std::filesystem::path& filePath) noexcept(false);

  /**
   * @copybrief addFile(const std::filesystem::path&, const std::filesystem::path&)
   * @param filePath File to add
   * @param data File data
   * @throw std::runtime_error
   */
  void addFile(const std::filesystem::path& filePath, const Buffer& data) noexcept(false);

  /**
   * @brief Returns a pointer to the file data for the specified path.
   * @param fileName File to look for
   * @return Pointer to file
   * @throw std::runtime_error
   */
  [[nodiscard]] FileRecord_t findFileRecord(const std::filesystem::path& fileName) noexcept;

  /**
   * @brief Extracts the specified file into a buffer.
   * @param fileRecord File to extract
   * @return Buffer containing the file data
   * @throw std::runtime_error
   */
  [[nodiscard]] Buffer extractFileData(const FileRecord_t& fileRecord) noexcept(false);

  /**
   * @brief Extracts the specified file into a buffer.
   * @param fileName File to extract
   * @return Buffer containing the file data
   * @throw std::runtime_error
   */
  [[nodiscard]] Buffer extractFileData(const std::filesystem::path& fileName) noexcept(false);

  /**
   * @brief Extracts the specified file and saves it to disk.
   * @param filePath File to extract
   * @param saveAs Target path
   * @throw std::runtime_error
   */
  void extractFile(const std::filesystem::path& filePath, const std::filesystem::path& saveAs) noexcept(false);

  /**
   * @brief Iterates over all files inside the archive and calls the provided function.
   * @param function Function to call
   * @param data Optional data to provide to function calls
   */
  void iterateFiles(const FileIterationFunction& function, void* data = nullptr) noexcept;

  /**
   * @brief Checks if a specified file exists inside the archive.
   */
  [[nodiscard]] bool fileExists(const std::filesystem::path& filePath) noexcept;

  /**
   * @brief Returns a vector containing all file paths inside the archive. If the optional parameter is provided, only
   * files inside the specified directory are returned.
   * @param directoryName Optionally restrict the file list to files inside this directory
   * @return Filelist
   */
  [[nodiscard]] std::vector<std::filesystem::path>
  getFileList(const std::filesystem::path& directoryName = {}) const noexcept;

  /**
   * @brief Returns the archive file name.
   */
  [[nodiscard]] std::filesystem::path getFileName() const noexcept;

  /**
   * @brief Returns the archive type.
   */
  [[nodiscard]] ArchiveType getArchiveType() const noexcept;

  /**
   * @brief Returns the header version. See HeaderVersion in enums.h
   */
  [[nodiscard]] HeaderVersion getVersion() const noexcept;

  /**
   * @brief Returns a string describing the archive format.
   */
  [[nodiscard]] std::string getArchiveFormatName() const noexcept;

  /**
   * @brief Returns the number of files inside the archive.
   */
  [[nodiscard]] uint32_t getFileCount() const noexcept;

  /**
   * @brief Returns the current size of a created archive.
   */
  [[nodiscard]] int64_t getCreatedArchiveSize() const noexcept;

  /**
   * @brief Returns archive flags. See ArchiveFlag in enums.h
   */
  [[nodiscard]] uint32_t getArchiveFlags() const noexcept;

  /**
   * @brief Sets archive flags.
   * @throw std::runtime_error If flags are not supported for the current archive type
   */
  void setArchiveFlags(uint32_t flags) noexcept(false);

  /**
   * @brief Returns whether archive data is compressed.
   */
  [[nodiscard]] bool getCompressed() const noexcept;

  /**
   * @brief Sets archive compression.
   */
  void setCompressed(bool value) noexcept;

  /**
   * @brief Returns whether shared data is enabled.
   */
  [[nodiscard]] bool getShareData() const noexcept;

  /**
   * @brief Sets shared data.
   */
  void setShareData(bool value) noexcept;

  /**
   * @brief Sets dds base path. This is required for FO4dds and SFdds archives.
   */
  void setDDSBasePath(const std::filesystem::path& path) noexcept;

  /**
   * @brief Returns whether multithreading is enabled.
   */
  [[nodiscard]] bool isMultithreaded() const noexcept;

  /**
   * @brief Sets multithreading. Only call this function before extracting or creating an archive.
   */
  void setMultithreading(bool value) noexcept;

  /**
   * @brief Returns compression level.
   */
  [[nodiscard]] int getCompressionLevel() const noexcept;

  /**
   * @brief Sets compression level.
   */
  void setCompressionLevel(int value) noexcept;

private:
  class BsaImpl;
  BsaImpl* m_impl;
};
}  // namespace libbsarchpp
