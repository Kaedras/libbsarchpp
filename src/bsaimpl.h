#pragma once

#include "archiveio.h"
#include "bsa.h"

#include <atomic>
#include <mutex>

namespace libbsarchpp {

class Bsa::BsaImpl {
public:
  static void extract(const std::filesystem::path& archivePath, const std::filesystem::path& outputDirectory,
                      bool multithreaded = false) noexcept(false);
  static void create(const std::filesystem::path& archivePath, ArchiveType type,
                     const std::filesystem::path& inputDirectory,
                     const BsaCreationSettings& settings = {}) noexcept(false);
  [[nodiscard]] static ArchiveType getArchiveType(const std::filesystem::path& archivePath) noexcept(false);

  explicit BsaImpl(const std::filesystem::path& archivePath, bool multithreaded = false) noexcept(false);

  BsaImpl(const std::filesystem::path& archivePath, ArchiveType type, std::vector<std::filesystem::path>& fileList,
          std::filesystem::path ddsBasePath, bool compressed, bool shareData, bool multithreaded) noexcept(false);
  BsaImpl(const std::filesystem::path& archivePath, ArchiveType type, std::vector<std::filesystem::path>& fileList,
          std::filesystem::path ddsBasePath) noexcept(false);

  ~BsaImpl();

  void save() noexcept(false);
  void addFile(const std::filesystem::path& rootDirectory, const std::filesystem::path& filePath) noexcept(false);
  void addFile(const std::filesystem::path& filePath, const Buffer& data) noexcept(false);
  [[nodiscard]] FileRecord_t findFileRecord(const std::filesystem::path& fileName) noexcept;
  [[nodiscard]] Buffer extractFileData(const FileRecord_t& fileRecord) noexcept(false);
  [[nodiscard]] Buffer extractFileData(const std::filesystem::path& fileName) noexcept(false);
  void extractFile(const std::filesystem::path& filePath, const std::filesystem::path& saveAs) noexcept(false);
  void iterateFiles(const FileIterationFunction& function, void* data) noexcept;
  [[nodiscard]] std::vector<std::filesystem::path>
  getFileList(const std::filesystem::path& directoryName = {}) const noexcept;
  [[nodiscard]] bool fileExists(const std::filesystem::path& filePath) noexcept;
  [[nodiscard]] std::filesystem::path getFileName() const noexcept;
  [[nodiscard]] ArchiveType getArchiveType() const noexcept;
  [[nodiscard]] HeaderVersion getVersion() const noexcept;
  [[nodiscard]] std::string getArchiveFormatName() const noexcept;
  [[nodiscard]] uint32_t getFileCount() const noexcept;
  [[nodiscard]] int64_t getCreatedArchiveSize() const noexcept;
  [[nodiscard]] uint32_t getArchiveFlags() const noexcept;
  void setArchiveFlags(uint32_t flags) noexcept(false);
  [[nodiscard]] bool getCompressed() const noexcept;
  void setCompressed(bool value) noexcept;
  [[nodiscard]] bool getShareData() const noexcept;
  void setShareData(bool value) noexcept;
  void setDDSBasePath(const std::filesystem::path& path) noexcept;
  [[nodiscard]] bool isMultithreaded() const noexcept;
  void setMultithreading(bool value) noexcept;
  [[nodiscard]] int getCompressionLevel() const noexcept;
  void setCompressionLevel(int value) noexcept;

private:
  bool m_existingArchive; /**< True when an already existing archive has been opened */
  std::filesystem::path m_fileName;
  ArchiveIO m_archiveFile;
  std::filesystem::path m_ddsBasePath;
  uint32_t m_magic                  = 0;
  uint32_t m_version                = 0;
  bool m_compressed                 = false;
  int m_compressionLevel            = 0;
  CompressionType m_compressionType = zlib;
  bool m_shareData                  = false;
  bool m_multithreaded              = false;
  /**
   * @brief When using multithreading, this variable is set to true to abort calls of addFile and extractFile
   */
  std::atomic<bool> m_abort = false;
  std::mutex m_writeMtx;
  std::mutex m_packedDataMtx;
  std::mutex m_fileMapMtx;

  std::vector<std::variant<FileTES3, FolderTES4, FileFO4>> m_files;
  std::variant<HeaderTES3, HeaderTES4, HeaderFO4, HeaderSF, HeaderSFdds> m_header;

  std::unordered_map<std::filesystem::path, FilePtr_t> m_fileMap;

  int32_t m_maxChunkCount   = 4;
  int32_t m_singleMipChunkX = 512;
  int32_t m_singleMipChunkY = 512;
  uint64_t m_dataOffset     = 0;
  std::vector<PackedDataInfo> m_packedData;

  /**
   * @brief Adds a file to @link m_fileMap @endlink.
   * @param filePath File path
   * @param file File pointer
   * @throw std::runtime_error
   */
  void addToFileMap(const std::filesystem::path& filePath, FilePtr_t file) noexcept(false);

  /**
   * @brief Calculates mip map chunk count from provided dds info.
   */
  int getDDSMipChunkCount(const DDSInfo& DDSInfo) const noexcept;

  /**
   * @brief Calculates MD5 of the provided buffer.
   * @param data Buffer
   * @param length Buffer length
   * @return MD5 of buffer
   */
  static md5sum calcDataHash(const uint8_t* data, size_t length) noexcept;

  /**
   * @brief Checks if there is an identical file in @link m_files @endlink. This also sets the size and offset of the
   * provided file.
   * @param size Data size
   * @param hash Data hash
   * @param fileRecord File record
   * @return Whether an identical file exists in @link m_files @endlink. Always returns false if @link m_shareData
   * @endlink is false.
   */
  bool findPackedData(size_t size, const md5sum& hash, const FileRecord_t& fileRecord) noexcept;

  /**
   * @brief Adds the provided file to @link m_packedData @endlink. Does not do anything if @link m_shareData @endlink is
   * false.
   * @param size Data size
   * @param hash Data hash
   * @param fileRecord File record
   */
  void addPackedData(uint32_t size, const md5sum& hash, const FileRecord_t& fileRecord) noexcept;

  /**
   * @brief Writes the provided data into the archive file.
   * @param fileRecord File record
   * @param filePath File path
   * @param dataHash File hash. Only used if @link m_shareData @endlink is true.
   * @param data File data
   * @param size Data size
   * @param compress Compress
   * @param doCompress Force compression
   * @throw std::runtime_error
   */
  void packData(const FileRecord_t& fileRecord, const std::filesystem::path& filePath, const md5sum& dataHash,
                const uint8_t* data, size_t size, bool compress, bool doCompress = false) noexcept(false);

  /**
   * @brief Compresses the provided buffer. Compression algorithm is dependent on the archive type.
   * @param data Uncompressed data
   * @return Compressed data
   * @throw std::runtime_error
   */
  [[nodiscard]] Buffer compressData(const Buffer& data) const noexcept(false);

  /**
   * @copybrief compressData(const Buffer&)
   * @param data Uncompressed buffer
   * @param length Buffer length
   * @return Compressed data
   * @throw std::runtime_error
   */
  [[nodiscard]] Buffer compressData(const uint8_t* data, size_t length) const noexcept(false);

  /**
   * @brief Decompresses the provided buffer.
   * @param data Compressed buffer
   * @param uncompressedSize Uncompressed buffer size
   * @return Uncompressed data
   * @throw std::runtime_error
   */
  [[nodiscard]] Buffer decompressData(const Buffer& data, uint32_t uncompressedSize) const noexcept(false);

  /**
   * @brief Decompresses the provided buffer into a provided output buffer. Memory must already be allocated.
   * @param compressed Compressed buffer
   * @param [out] uncompressed Uncompressed buffer
   * @param uncompressedSize Uncompressed buffer size
   * @throw std::runtime_error
   */
  void decompressData(const Buffer& compressed, uint8_t* uncompressed, uint32_t uncompressedSize) const noexcept(false);

  /**
   * @brief Reads dds info from the provided file.
   * @param filePath Path to dds file
   * @throw std::runtime_error
   */
  [[nodiscard]] DDSInfo getDDSInfo(const std::filesystem::path& filePath) const noexcept(false);

  /**
   * @brief Locks @link m_writeMtx @endlink if @link m_multithreaded @endlink is true.
   */
  void lock() noexcept;

  /**
   * @brief Unlocks @link m_writeMtx @endlink if @link m_multithreaded @endlink is true.
   */
  void unlock() noexcept;

  void seek(int64_t pos, SeekDirection whence = SET) const noexcept(false) { m_archiveFile.seek(pos, whence); }

  /**
   * @brief Returns HeaderFO4 structure from @link m_header @endlink.
   * @throw std::runtime_error If the archive does not contain a FO4 header
   */
  HeaderFO4& getHeaderFO4() noexcept(false);

  /**
   * @brief This function is called by @link Bsa::findFileRecord @endlink if the archive type is TES4.
   */
  [[nodiscard]] FileRecord_t findFileRecordTES4(const std::filesystem::path& filePath) noexcept;

  /**
   * @brief Creates a TES3 archive.
   * @throw std::runtime_error
   */
  void createArchiveTES3(std::vector<std::filesystem::path>& fileList) noexcept(false);

  /**
   * @brief Creates a TES4 archive.
   * @throw std::runtime_error
   */
  void createArchiveTES4(std::vector<std::filesystem::path>& fileList) noexcept(false);

  /**
   * @brief Creates a FO4 archive.
   * @throw std::runtime_error
   */
  void createArchiveFO4(std::vector<std::filesystem::path>& fileList) noexcept(false);

  /**
   * @brief Reads a TES3 archive.
   * @throw std::runtime_error
   */
  void readArchiveTes3() noexcept(false);

  /**
   * @brief Reads a TES4 archive.
   * @throw std::runtime_error
   */
  void readArchiveTes4() noexcept(false);

  /**
   * @brief Reads file table from a FO4, FO4dds, SF, or SFdds archive.
   * @throw std::runtime_error
   */
  void readFO4FileTable() noexcept(false);

  /**
   * @brief Reads general files from a FO4 or SF archive.
   * @throw std::runtime_error
   */
  void readBa2GNRL() noexcept(false);

  /**
   * @brief Reads texture files from a FO4dds or SFdds archive.
   * @throw std::runtime_error
   */
  void readBa2DX10() noexcept(false);

  /**
   * @brief Determines the exact archive version.
   * @throw std::runtime_error
   */
  void determineArchiveVersion() noexcept(false);

  /**
   * @brief This function is called by @link Bsa::addFile @endlink if the archive type is FO4dds or SFdds.
   * @throw std::runtime_error
   */
  void addFileDDS(FileFO4* file, const Buffer& data) noexcept(false);

  template <typename T>
  T read() noexcept(false) {
    return m_archiveFile.read<T>();
  }

  template <typename T>
  std::vector<T> read(uint32_t length) noexcept(false) {
    return m_archiveFile.read<T>(length);
  }
  template <typename T>
  void read(T* data, uint32_t length) noexcept(false) {
    return m_archiveFile.read<T>(data, length);
  }

  template <typename T>
  void write(const T& data) noexcept(false) {
    m_archiveFile.write<T>(data);
  }
  template <typename T>
  void write(const T* data, size_t length) noexcept(false) {
    m_archiveFile.write<T>(data, length);
  }
};
}  // namespace libbsarchpp
