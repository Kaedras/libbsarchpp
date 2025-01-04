#pragma once

#include "directx/dxgiformat.h"
#include "dllexport.h"
#include "enums.h"
#include "types.h"
#include <atomic>
#include <bit>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <functional>
#include <mutex>
#include <openssl/types.h>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

static_assert(std::endian::native == std::endian::little);

namespace libbsarchpp {

using std::string_literals::operator""s;
using FileIterationFunction = std::function<
    bool(const std::filesystem::path &, FilePtr_t, FolderTES4 *, void *)>; // filename, file, folder, optional data

/**
 * Class to handle Bethesda archives
 */
class DLL_PUBLIC Bsa
{
public:
    /**
     * @brief Extracts an archive file to a specified output directory.
     * @param archivePath Archive to extract
     * @param outputDirectory Output directory
     * @param multithreaded Enable multithreading
     * @throw std::runtime_error
     */
    static void extract(const std::filesystem::path &archivePath,
                        const std::filesystem::path &outputDirectory,
                        bool multithreaded = false) noexcept(false);

    /**
     * @brief Creates a new archive with specified type from files inside a specified input directory.
     * @param archivePath Output file path
     * @param type Archive type
     * @param inputDirectory Base directory of input files
     * @param settings Settings to use when creating archive
     * @throw std::runtime_error
     */
    static void create(const std::filesystem::path &archivePath,
                       ArchiveType type,
                       const std::filesystem::path &inputDirectory,
                       BsaCreationSettings settings = {}) noexcept(false);

    /**
     * @brief Reads the header of specified archive file and returns the archive type.
     * @param archivePath Archive to check
     * @return Type of archive
     * @throw std::runtime_error
     */
    [[nodiscard]] static ArchiveType getArchiveType(const std::filesystem::path &archivePath) noexcept(false);

    /**
     * @brief Opens an existing archive.
     * @param archivePath Archive to open
     * @param multithreaded Enable multithreaded extraction
     * @throw std::runtime_error
     */
    explicit Bsa(const std::filesystem::path &archivePath, bool multithreaded = false) noexcept(false);

    /**
     * @brief Creates a new archive.
     * @param archivePath Archive file to create
     * @param type Archive type
     * @param fileList Predefined list of files
     * @param ddsBasePath Base path for dds files, only required for FO4dds and SFdds archives
     * @param compressed Enable compression
     * @param shareData Identical files will only be written once. Potentially reduces filesize while reducing performance
     * @param multithreaded Enable multithreaded packing. Increases performance, but produces indeterministic results
     * @throw std::runtime_error
     */
    Bsa(const std::filesystem::path &archivePath,
        ArchiveType type,
        std::vector<std::filesystem::path> &fileList,
        const std::optional<std::filesystem::path> &ddsBasePath,
        bool compressed,
        bool shareData,
        bool multithreaded) noexcept(false);

    /**
     * @copybrief Bsa(const std::filesystem::path&, ArchiveType, std::vector<std::filesystem::path>&, const std::optional<std::filesystem::path>&, bool, bool, bool)
     * @param archivePath Archive file to create
     * @param type Archive type
     * @param fileList Predefined list of files
     * @param ddsBasePath Base path for dds files, only required for FO4dds and SFdds archives
     * @throw std::runtime_error
     */
    Bsa(const std::filesystem::path &archivePath,
        ArchiveType type,
        std::vector<std::filesystem::path> &fileList,
        const std::optional<std::filesystem::path> &ddsBasePath = std::nullopt) noexcept(false);

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
    void addFile(const std::filesystem::path &rootDirectory, const std::filesystem::path &filePath) noexcept(false);

    /**
     * @copybrief addFile(const std::filesystem::path&, const std::filesystem::path&)
     * @param filePath File to add
     * @param data File data
     * @throw std::runtime_error
     */
    void addFile(const std::filesystem::path &filePath, Buffer &data) noexcept(false);

    /**
     * @brief Returns a pointer to the file data for the specified path.
     * @param fileName File to look for
     * @return Pointer to file
     * @throw std::runtime_error
     */
    [[nodiscard]] FileRecord_t findFileRecord(const std::filesystem::path &fileName) noexcept;

    /**
     * @brief Extracts the specified file into a buffer.
     * @param fileRecord File to extract
     * @return Buffer containing the file data
     * @throw std::runtime_error
     */
    [[nodiscard]] Buffer extractFileData(const FileRecord_t &fileRecord) noexcept(false);

    /**
     * @brief Extracts the specified file into a buffer.
     * @param fileName File to extract
     * @return Buffer containing the file data
     * @throw std::runtime_error
     */
    [[nodiscard]] Buffer extractFileData(const std::filesystem::path &fileName) noexcept(false);

    /**
     * @brief Extracts the specified file and saves it to disk.
     * @param filePath File to extract
     * @param saveAs Target path
     * @throw std::runtime_error
     */
    void extractFile(const std::filesystem::path &filePath, const std::filesystem::path &saveAs) noexcept(false);

    /**
     * @brief Iterates over all files inside the archive and calls the provided function.
     * @param function Function to call
     * @param data Optional data to provide to function calls
     */
    void iterateFiles(const FileIterationFunction &function, void *data = nullptr) noexcept;

    /**
     * @brief Checks if a specified file exists inside the archive.
     */
    [[nodiscard]] bool fileExists(const std::filesystem::path &filePath) noexcept;

    /**
     * @brief Returns a vector containing all file paths inside the archive. If the optional parameter is provided, only files inside the specified directory are returned.
     * @param directoryName Optionally restrict file list to files inside this directory
     * @return Filelist
     */
    [[nodiscard]] std::vector<std::filesystem::path> getFileList(
        const std::optional<std::filesystem::path> &directoryName = std::nullopt) const noexcept;

    /**
     * @brief Returns the archive file name.
     */
    [[nodiscard]] std::filesystem::path getFileName() const noexcept;

    /**
     * @brief Returns the archive type.
     */
    [[nodiscard]] ArchiveType getArchiveType() const noexcept;

    /**
     * @brief Returns the header version. Values can be found in namespace headerVersions inside constants.h.
     */
    [[nodiscard]] uint32_t getVersion() const noexcept;

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
    * @brief Returns archive flags. A list of flags can be found in namespace flags::archive inside constants.h.
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
    void setDDSBasePath(const std::filesystem::path &path) noexcept;

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
    std::unique_ptr<FILE, fileDeleter> m_file;
    bool m_existingArchive; /**< True when an already existing archive has been opened */
    ArchiveType m_type = none;
    std::filesystem::path m_fileName;
    std::filesystem::path m_ddsBasePath;
    uint32_t m_magic = 0;
    uint32_t m_version = 0;
    bool m_compressed = false;
    int m_compressionLevel = 0;
    CompressionType m_compressionType = zlib;
    bool m_shareData = false;
    bool m_multithreaded = false;
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

    int32_t m_maxChunkCount = 4;
    int32_t m_singleMipChunkX = 512;
    int32_t m_singleMipChunkY = 512;
    uint64_t m_dataOffset = 0;
    std::vector<PackedDataInfo> m_packedData;

    EVP_MD *m_md = nullptr;

    /**
     * @brief Adds a file to @link m_fileMap @endlink.
     * @param filePath File path
     * @param file File pointer
     * @throw std::runtime_error
     */
    void addToFileMap(const std::filesystem::path &filePath, FilePtr_t file) noexcept(false);

    /**
     * @brief Calculates mip map chunk count from provided dds info.
     */
    int getDDSMipChunkCount(const DDSInfo &DDSInfo) const noexcept;

    /**
     * @brief Calculates MD5 of provided buffer.
     * @param data Buffer
     * @param length Buffer length
     * @return MD5 of buffer
     */
    PackedDataHash calcDataHash(const uint8_t *data, uint32_t length) const noexcept;

    /**
     * @brief Checks if there is an identical file in @link m_files @endlink. This also sets size and offset of the provided file.
     * @param size Data size
     * @param hash Data hash
     * @param fileRecord File record
     * @return Whether identical file exists in @link m_files @endlink. Always returns false if @link m_shareData @endlink is false.
     */
    bool findPackedData(uint32_t size, const PackedDataHash &hash, const FileRecord_t &fileRecord) noexcept;

    /**
     * @brief Adds provided file to @link m_packedData @endlink. Does not do anything if @link m_shareData @endlink is false.
     * @param size Data size
     * @param hash Data hash
     * @param fileRecord File record
     */
    void addPackedData(uint32_t size, const PackedDataHash &hash, const FileRecord_t &fileRecord) noexcept;

    /**
     * @brief Writes the provided data into the archive file.
     * @param fileRecord File record
     * @param filePath File path
     * @param dataHash File hash. Only used if @link m_shareData @endlink is true.
     * @param data File data
     * @param compress Compress
     * @param doCompress Force compression
     * @throw std::runtime_error
     */
    void packData(const FileRecord_t &fileRecord,
                  const std::filesystem::path &filePath,
                  const PackedDataHash &dataHash,
                  const uint8_t *data,
                  uint32_t size,
                  bool compress,
                  bool doCompress = false) noexcept(false);

    /**
     * @brief Compresses the provided buffer. Compression algorithm is dependent on archive type.
     * @param data Uncompressed data
     * @return Compressed data
     * @throw std::runtime_error
     */
    [[nodiscard]] Buffer compressData(const Buffer &data) const noexcept(false);

    /**
     * @brief Compresses the provided buffer. Compression algorithm is dependent on archive type.
     * @param data Uncompressed buffer
     * @param length Buffer length
     * @return Compressed data
     * @throw std::runtime_error
     */
    [[nodiscard]] Buffer compressData(const uint8_t *data, size_t length) const noexcept(false);

    /**
     * @brief Decompresses the provided buffer.
     * @param data Compressed buffer
     * @param uncompressedSize Uncompressed buffer size
     * @return Uncompressed data
     * @throw std::runtime_error
     */
    [[nodiscard]] Buffer decompressData(const Buffer &data, uint32_t uncompressedSize) const noexcept(false);

    /**
     * @brief Decompresses the provided buffer into a provided output buffer. Memory must already be allocated.
     * @param compressed Compressed buffer
     * @param [out] uncompressed Uncompressed buffer
     * @param uncompressedSize Uncompressed buffer size
     * @throw std::runtime_error
     */
    void decompressData(const Buffer& compressed, uint8_t* uncompressed, uint32_t uncompressedSize) const noexcept(false);

    /**
     * @brief Reads dds info from provided file.
     * @param filePath Path to dds file
     * @throw std::runtime_error
     */
    [[nodiscard]] DDSInfo getDDSInfo(const std::filesystem::path &filePath) const noexcept(false);

    /**
     * @brief Locks @link m_writeMtx @endlink if @link m_multithreaded @endlink is true.
     */
    void lock() noexcept;

    /**
     * @brief Unlocks @link m_writeMtx @endlink if @link m_multithreaded @endlink is true.
     */
    void unlock() noexcept;

    /**
     * @brief Returns whether strings written to file should contain backslashes instead of slashes.
     */
    bool useBackslashes() const noexcept
    {
        return m_type != FO4 && m_type != FO4dds && m_type != SF && m_type != SFdds;
    }

    /**
     * @brief Calls fseek and checks for errors.
     * @param pos Position to seek to
     * @param whence Seek direction
     * @throw std::runtime_error
     */
    void seek(int64_t pos, SeekDirection whence = SET) const noexcept(false);

    /**
     * @brief Returns HeaderFO4 structure from @link m_header @endlink.
     * @throw std::runtime_error If archive does not contain a FO4 header
     */
    HeaderFO4 &getHeaderFO4() noexcept(false);

    /**
     * @brief This function is called by @link Bsa::findFileRecord @endlink if archive type is TES4.
     */
    [[nodiscard]] FileRecord_t findFileRecordTES4(const std::filesystem::path &filePath) noexcept;

    /**
     * @brief Creates a TES3 archive.
     * @throw std::runtime_error
     */
    void createArchiveTES3(std::vector<std::filesystem::path> &fileList) noexcept(false);

    /**
     * @brief Creates a TES4 archive.
     * @throw std::runtime_error
     */
    void createArchiveTES4(std::vector<std::filesystem::path> &fileList) noexcept(false);

    /**
     * @brief Creates a FO4 archive.
     * @throw std::runtime_error
     */
    void createArchiveFO4(std::vector<std::filesystem::path> &fileList) noexcept(false);

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
     * @brief Determines exact archive version.
     * @throw std::runtime_error
     */
    void determineArchiveVersion() noexcept(false);

    /**
     * @brief This function is called by @link Bsa::addFile @endlink if archive type is FO4dds or SFdds.
     * @throw std::runtime_error
     */
    void addFileDDS(FileFO4 *file, const Buffer &data) noexcept(false);

    /**
     * @brief Returns the bits per pixel for the given format.
     * @throw std::runtime_error
     */
    static int bitsPerPixel(uint8_t format) noexcept(false);

    /**
     * @brief Returns the dxgi format of the given file.
     * @param data Buffer containing file data.
     * @throw std::runtime_error
     */
    static DXGI_FORMAT getDxgiFormat(const Buffer &data) noexcept(false);

    /**
     * @name IO Functions
     */
    ///@{
    /**
     * @brief Reads a value from file.
     * @tparam T Data type to read
     * @return Read value
     * @throw std::runtime_error
     */
    template<typename T>
    T read() noexcept(false)
    {
        T retVal;

        if (fread(&retVal, sizeof(T), 1, m_file.get()) == 1)
        {
            return retVal;
        }

        if (feof(m_file.get()))
        {
            throw std::runtime_error("Read error: EOF");
        }

        throw std::runtime_error("Read error: "s + strerror(errno));
    }

    /**
     * @brief Reads an array from file.
     * @param length array length
     * @return std::vector<@p T> containing the read data
     * @throw std::runtime_error
     */
    template<typename T>
    std::vector<T> read(uint32_t length) noexcept(false)
    {
        std::vector<T> retVal(length);

        if (fread(retVal.data(), sizeof(T), length, m_file.get()) == length)
        {
            return retVal;
        }

        const int error = errno;
        if (feof(m_file.get()))
        {
            throw std::runtime_error("Read error: EOF");
        }
        throw std::runtime_error("Read error: "s + strerror(error));
    }

    /**
     * @brief Reads an array from file into an existing array. Memory must already be allocated.
     * @param data Array to write into
     * @param length Elements to read
     * @throw std::runtime_error
     */
    template<typename T>
    void read(T* data, uint32_t length) noexcept(false)
    {
        size_t result = fread(data, sizeof(T), length, m_file.get());
        if (result != length)
        {
            const int error = errno;
            if (feof(m_file.get()))
            {
                throw std::runtime_error("Read error: EOF");
            }
            throw std::runtime_error("Read error: "s + strerror(error));
        }
    }

    /**
     * @brief Reads an u16string.
     * @param length String length
     * @throw std::runtime_error
     */
    std::u16string readU16String(uint32_t length) noexcept(false);

    /**
     * @brief Reads a string that starts with an uint8_t indicating the length.
     * @param terminated Whether string is null terminated
     * @throw std::runtime_error
     */
    std::filesystem::path readStringLen(bool terminated = true) noexcept(false);

    /**
     * @brief Reads a file name that starts with an uint16_t indicating the length.
     * @throw std::runtime_error
     */
    std::filesystem::path readStringLen16() noexcept(false);

    /**
     * @brief Writes data to file.
     * @throw std::runtime_error
     */
    template<typename T>
    void write(const T &data) noexcept(false)
    {
        if (fwrite(&data, sizeof(T), 1, m_file.get()) != 1)
        {
            throw std::runtime_error("Write error: "s + strerror(errno));
        }
    }

    /**
     * @brief Writes data to file.
     * @throw std::runtime_error
     */
    template<typename T>
    void write(const T* data, size_t length) noexcept(false)
    {
        if (fwrite(data, sizeof(T), length, m_file.get()) != length)
        {
            throw std::runtime_error("Write error: "s + strerror(errno));
        }
    }

    /**
     * @brief Writes data to file.
     * @throw std::runtime_error
     */
    template<typename T>
    void write(const std::vector<T> &data) noexcept(false)
    {
        if (fwrite(data.data(), sizeof(T), data.size(), m_file.get()) != data.size())
        {
            throw std::runtime_error("Write error: "s + strerror(errno));
        }
    }

    /**
     * @brief Writes a string that starts with an uint8_t indicating the length.
     * @param data String to write
     * @param terminated Write null-terminated string
     * @throw std::runtime_error
     */
    void writeStringLen8(const std::filesystem::path &data, bool terminated = true) noexcept(false);

    /**
     * @brief Writes a string that starts with an uint16_t indicating the length.
     * @throw std::runtime_error
     */
    void writeStringLen16(const std::filesystem::path &data) noexcept(false);

    ///@}
};
} // namespace libbsarchpp