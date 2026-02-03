#include "Bsa.h"

#include "constants.h"
#include "enums.h"
#include "hash.h"
#include "types.h"
#include "utils.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <compare>
#include <cstring>
#include <directx/dxgiformat.h>
#include <exception>
#include <execution>
#include <format>
#include <gsl-lite/gsl-lite.hpp>
#include <iostream>
#include <lz4.h>
#include <lz4frame.h>
#include <memory>
#include <mutex>
#include <openssl/evp.h>
#include <utility>
#include <variant>
#include <zconf.h>
#include <zlib.h>

namespace libbsarchpp {

#ifdef __unix__
// On Windows long is only 32-bit, so we can't use ftell there. On POSIX systems it's 64-bit
namespace {
long _ftelli64(FILE *stream)
{
    return ftell(stream);
}
// same with fseek
int _fseeki64(FILE *stream, long offset, int whence)
{
    return fseek(stream, offset, whence);
}
} // namespace
#endif

using namespace std;
namespace fs = std::filesystem;

template<>
char Bsa::read() noexcept(false)
{
    int result = fgetc(m_file.get());
    if (result == EOF)
    {
        const int error = errno;
        throw runtime_error("Read error: "s + strerror(error));
    }
    return static_cast<char>(result);
}

template<>
char8_t Bsa::read() noexcept(false)
{
    int result = fgetc(m_file.get());
    if (result == EOF)
    {
        const int error = errno;
        throw runtime_error("Read error: "s + strerror(error));
    }
    return static_cast<char8_t>(result);
}

template<>
Magic4 Bsa::read() noexcept(false)
{
    Magic4 result{};
    size_t itemsRead = fread(result.data(), 1, 4, m_file.get());
    if (itemsRead != 4)
    {
        const int error = errno;
        throw runtime_error("Read error: "s + strerror(error));
    }
    return result;
}

template<>
std::filesystem::path Bsa::read<>() noexcept(false)
{
    try
    {
        u16string str;

        char16_t c;
        do
        {
            c = read<uint8_t>();

            // we use forward slashes internally, this is required for std::filesystem
            if (c == '\\')
            {
                c = '/';
            }

            str += c;
        } while (c != '\0');

        str.pop_back();

        return {str};
    }
    catch (...)
    {
        throw;
    }
}

std::u16string Bsa::readU16String(const uint32_t length) noexcept(false)
{
    try
    {
        u16string str;

        for (uint32_t i = 0; i < length; i++)
        {
            char16_t c = read<uint8_t>();

            // we use forward slashes internally, this is required for std::filesystem
            if (c == '\\')
            {
                c = '/';
            }
            str += c;
        }

        return str;
    }
    catch (...)
    {
        throw;
    }
}

std::filesystem::path Bsa::readStringLen(const bool terminated) noexcept(false)
{
    try
    {
        const auto length = read<uint8_t>();
        auto str = readU16String(length);
        if (terminated)
        {
            str.pop_back();
        }
        return {str};
    }
    catch (...)
    {
        throw;
    }
}

std::filesystem::path Bsa::readStringLen16() noexcept(false)
{
    try
    {
        const auto length = read<uint16_t>();
        auto str = readU16String(length);
        return {str};
    }
    catch (...)
    {
        throw;
    }
}

template<>
void Bsa::write(const std::string &data) noexcept(false)
{
    try
    {
        // only call this function once, the result won't change
        static bool shouldUseBackslashes = useBackslashes();

        for (char c : data)
        {
            if (c == '/' && shouldUseBackslashes)
            {
                c = '\\';
            }
            write(c);
        }
        if (fputc('\0', m_file.get()) == EOF)
        {
            const int error = errno;
            throw runtime_error("Write error: "s + strerror(error));
        }
    }
    catch (...)
    {
        throw;
    }
}

template<>
void Bsa::write(const std::filesystem::path &data) noexcept(false)
{
    try
    {
        // only call this function once, the result won't change
        static bool shouldUseBackslashes = useBackslashes();

        u16string str = data.generic_u16string();

        // we use forward slashes internally, so we have to change them when writing
        if (shouldUseBackslashes)
        {
            changeSlashesToBackslashes(str);
        }

        for (const auto &c : str)
        {
            if (fputc(gsl_lite::narrow<uint8_t>(c), m_file.get()) == EOF)
            {
                const int error = errno;
                throw runtime_error("Read error: "s + strerror(error));
            }
        }
        if (fputc('\0', m_file.get()) == EOF)
        {
            const int error = errno;
            throw runtime_error("Read error: "s + strerror(error));
        }
    }
    catch (const gsl_lite::narrowing_error &ex)
    {
        throw runtime_error(ex.what());
    }
}

void Bsa::writeStringLen8(const std::filesystem::path &data, const bool terminated) noexcept(false)
{
    try
    {
        // only call this function once, the result won't change
        static bool shouldUseBackslashes = useBackslashes();

        u16string str = data.generic_u16string();
        auto length = gsl_lite::narrow<uint8_t>(str.length());
        if (terminated)
        {
            length++;
        }
        write(length);

        // we use forward slashes internally, so we have to change them when writing
        if (shouldUseBackslashes)
        {
            changeSlashesToBackslashes(str);
        }

        for (const auto &c : str)
        {
            if (fputc(gsl_lite::narrow<uint8_t>(c), m_file.get()) == EOF)
            {
                const int error = errno;
                throw runtime_error("Error writing to file: "s + strerror(error));
            }
        }
        if (terminated)
        {
            if (fputc('\0', m_file.get()) == EOF)
            {
                const int error = errno;
                throw runtime_error("Error writing to file: "s + strerror(error));
            }
        }
    }
    catch (const gsl_lite::narrowing_error &ex)
    {
        throw runtime_error(ex.what());
    }
    catch (...)
    {
        throw;
    }
}

void Bsa::writeStringLen16(const std::filesystem::path &data) noexcept(false)
{
    try
    {
        // only call this function once, the result won't change
        static bool shouldUseBackslashes = useBackslashes();

        u16string str = data.generic_u16string();
        write(gsl_lite::narrow<uint16_t>(str.length()));

        // we use forward slashes internally, so we have to change them when writing
        if (shouldUseBackslashes)
        {
            changeSlashesToBackslashes(str);
        }

        for (const auto &c : str)
        {
            if (fputc(gsl_lite::narrow<uint8_t>(c), m_file.get()) == EOF)
            {
                const int error = errno;
                throw runtime_error("Write error: "s + strerror(error));
            }
        }
    }
    catch (const gsl_lite::narrowing_error &ex)
    {
        throw runtime_error(ex.what());
    }
    catch (...)
    {
        throw;
    }
}

uint32_t Bsa::getFileCount() const noexcept
{
    try
    {
        switch (m_type)
        {
            case TES3: return get<HeaderTES3>(m_header).fileCount;
            case TES4:
            case FO3:
            case SSE: return get<HeaderTES4>(m_header).fileCount;
            case FO4:
            case FO4dds: return get<HeaderFO4>(m_header).fileCount;
            case SF: return get<HeaderSF>(m_header).fo4Header.fileCount;
            case SFdds: return get<HeaderSFdds>(m_header).fo4Header.fileCount;
            default: return 0;
        }
    }
    catch ([[maybe_unused]] const bad_variant_access &ex)
    {
        return 0;
    }
}

void Bsa::setArchiveFlags(uint32_t flags) noexcept(false)
{
    try
    {
        if (m_type != TES4 && m_type != FO3 && m_type != SSE)
        {
            throw runtime_error("Archive flags are not supported for this archive type");
        }

        // force compression flag if needed
        if (m_compressed)
        {
            flags |= flags::archive::COMPRESS;
        }

        get<HeaderTES4>(m_header).flags = flags;
    }
    catch (const bad_variant_access &ex)
    {
        throw runtime_error("Internal error! Bad variant access: "s + ex.what());
    }
}

bool Bsa::getCompressed() const noexcept
{
    return m_compressed;
}

void Bsa::setCompressed(const bool value) noexcept
{
    m_compressed = value;
}

bool Bsa::getShareData() const noexcept
{
    return m_shareData;
}

void Bsa::setShareData(const bool value) noexcept
{
    m_shareData = value;
}

void Bsa::setDDSBasePath(const std::filesystem::path &path) noexcept
{
    m_ddsBasePath = path;
}

bool Bsa::isMultithreaded() const noexcept
{
    return m_multithreaded;
}

void Bsa::setMultithreading(bool value) noexcept
{
    m_multithreaded = value;
}

int Bsa::getCompressionLevel() const noexcept
{
    return m_compressionLevel;
}

void Bsa::setCompressionLevel(int value) noexcept
{
    if (m_compressionType == zlib)
    {
        // check if compression level is valid
        if (value >= Z_BEST_SPEED && value <= Z_BEST_COMPRESSION)
        {
            m_compressionLevel = value;
        }
        else if (m_compressionLevel > Z_BEST_COMPRESSION)
        {
            // set to Z_BEST_COMPRESSION if value is too large
            m_compressionLevel = Z_BEST_COMPRESSION;
        }
        else
        {
            // set to Z_DEFAULT_COMPRESSION if value is too small
            m_compressionLevel = Z_DEFAULT_COMPRESSION;
        }
    }
    else
    {
        // lz4 handles invalid compression levels internally, so no check is required here
        m_compressionLevel = value;
    }
}

ArchiveType Bsa::getArchiveType(const std::filesystem::path &archivePath) noexcept(false)
{
    try
    {
        Bsa bsa(archivePath);
        return bsa.getArchiveType();
    }
    catch (...)
    {
        throw;
    }
}

void Bsa::readFO4FileTable() noexcept(false)
{
    try
    {
        const HeaderFO4 &headerFO4 = getHeaderFO4();
        // read file names
        seek(headerFO4.fileTableOffset);

        // this map stores all directories, key is lowercase
        unordered_map<string, string> pathMap;

        for (auto &_file : m_files)
        {
            // the code below is required for case-sensitive file systems
            fs::path filePath = readStringLen16();
            string parentPath = filePath.parent_path().string();

            size_t oldPos = 0;
            size_t pos = 0;

            // iterate over all subfolders starting at root directory
            // if a folder already exists in pathMap replace the path with the one stored there
            while (pos != string::npos)
            {
                oldPos = pos;
                pos = parentPath.find('/', oldPos + 1);

                string subStr = parentPath.substr(0, pos);

                const auto [iterator, wasInserted] = pathMap.try_emplace(ToLower(subStr), subStr);
                if (!wasInserted)
                {
                    // directory already exists in map
                    size_t charsToReplace = pos - oldPos;
                    parentPath.replace(oldPos, charsToReplace, iterator->second, oldPos, charsToReplace);
                }
            }

            auto &file = get<FileFO4>(_file);
            file.name = fs::path(parentPath) / filePath.filename();

            addToFileMap(file.name, &file);
        }
    }
    catch (...)
    {
        throw;
    }
}

void Bsa::readBa2GNRL() noexcept(false)
{
    const HeaderFO4 &header = getHeaderFO4();

    if (header.magic != magic::GNRL)
    {
        throw runtime_error("Invalid/Unknown ba2 archive. Expected GNRL, got " + MagicToString(header.magic));
    }

    m_files.reserve(header.fileCount);
    for (uint32_t i = 0; i < header.fileCount; i++)
    {
        FileFO4 file;

        file.nameHash = read<uint32_t>();
        file.ext = read<Magic4>();
        file.dirHash = read<uint32_t>();
        file.unknown = read<uint32_t>();
        file.offset = read<int64_t>();
        file.packedSize = read<uint32_t>();
        file.size = read<uint32_t>();

        // next value is a constant, so we can use it to check for errors
        if (auto footer = read<uint32_t>(); footer != FileFO4Footer)
        {
            throw runtime_error(format("Read error: expected 0xBAADF00D, got {:#x}", footer));
        }

        m_files.emplace_back(file);
    }
}

void Bsa::readBa2DX10() noexcept(false)
{
    const HeaderFO4 &header = getHeaderFO4();
    if (header.magic != magic::DX10)
    {
        throw runtime_error("Invalid/Unknown ba2 archive. Expected GNRL, got " + MagicToString(header.magic));
    }

    if (m_type == FO4)
    {
        m_type = FO4dds;
    }
    else if (m_type == SF)
    {
        m_type = SFdds;
    }

    m_files.reserve(header.fileCount);
    for (uint32_t i = 0; i < header.fileCount; i++)
    {
        FileFO4 file;

        file.nameHash = read<uint32_t>();
        file.ext = read<Magic4>();
        file.dirHash = read<uint32_t>();
        file.unknownTex = read<uint8_t>();

        auto texChunkSize = read<uint8_t>();
        file.texChunks.reserve(texChunkSize);
        // next value is a constant, so we can use it to check for errors
        if (auto chunkHeaderSize = read<uint16_t>(); chunkHeaderSize != sizes::chunkHeader)
        {
            throw runtime_error(format("Read error: expected 0x0018, got {:#x}", sizes::chunkHeader));
        }

        file.height = read<uint16_t>();
        file.width = read<uint16_t>();
        file.numMips = read<uint8_t>();
        file.dxgiFormat = read<uint8_t>();
        file.cubeMaps = read<uint16_t>();
        for (uint8_t j = 0; j < texChunkSize; j++)
        {
            file.texChunks.emplace_back(read<TexChunkRec>());
            // next value is a constant, so we can use it to check for errors
            if (auto footer = read<uint32_t>(); footer != FileFO4Footer)
            {
                throw runtime_error(format("Read error: expected 0xBAADF00D, got {:#x}", footer));
            }
        }

        m_files.emplace_back(file);
    }
}

void Bsa::readArchiveTes3() noexcept(false)
{
    // read header
    m_header = read<HeaderTES3>();
    const auto &headerTES3 = get<HeaderTES3>(m_header);

    // read file info
    m_files.reserve(headerTES3.fileCount);
    for (uint32_t i = 0; i < headerTES3.fileCount; i++)
    {
        FileTES3 file;
        file.size = read<uint32_t>();
        file.offset = read<uint32_t>();
        m_files.emplace_back(file);
    }
    // skip name offsets
    seek(4L * headerTES3.fileCount, CUR);
    // read names
    for (auto &_file : m_files)
    {
        auto &file = std::get<FileTES3>(_file);
        file.name = read<fs::path>();
        addToFileMap(file.name, &file);
    }
    // read hashes
    for (auto &file : m_files)
    {
        std::get<FileTES3>(file).hash = read<uint64_t>();
    }
    // remember binary data offset since stored files offsets are relative
    m_dataOffset = _ftelli64(m_file.get());
}

void Bsa::readArchiveTes4() noexcept(false)
{
    if (m_type == SSE)
    {
        m_compressionType = lz4Frame;
    }
    // read header
    m_header = read<HeaderTES4>();
    const auto &headerTES4 = get<HeaderTES4>(m_header);

    seek(headerTES4.foldersOffset);

    // read folder records
    m_files.reserve(headerTES4.folderCount);
    for (uint32_t i = 0; i < headerTES4.folderCount; i++)
    {
        FolderTES4 folder;

        folder.hash = read<uint64_t>();
        folder.fileCount = read<uint32_t>();
        if (m_type == SSE)
        {
            folder.unk32 = read<uint32_t>();
            folder.offset = read<int64_t>();
        }
        else
        {
            folder.offset = read<uint32_t>();
        }

        m_files.emplace_back(folder);
    }

    // read folder names and file records
    for (auto &_folder : m_files)
    {
        auto &folder = get<FolderTES4>(_folder);

        folder.name = readStringLen();
        folder.files.reserve(folder.fileCount);
        for (uint32_t j = 0; j < folder.fileCount; j++)
        {
            FileTES4 file;
            file.hash = read<uint64_t>();
            file.size = read<uint32_t>();
            file.offset = read<uint32_t>();
            folder.files.emplace_back(file);
        }
    }

    // read file names
    for (auto &folder : m_files)
    {
        for (auto &file : get<FolderTES4>(folder).files)
        {
            file.name = read<fs::path>();
        }
    }
}

void Bsa::determineArchiveVersion() noexcept(false)
{
    if (m_type == TES3)
    {
        return;
    }

    m_version = read<uint32_t>();
    switch (m_version)
    {
        case headerVersions::TES4: m_type = TES4; break;
        case headerVersions::FO3: m_type = FO3; break;
        case headerVersions::SSE: m_type = SSE; break;
        case headerVersions::FO4v1:
        case headerVersions::FO4NGv7:
        case headerVersions::FO4NGv8: {
            // read ahead to get subtype
            auto magic = read<uint32_t>();
            if (magic == magic::GNRL)
            {
                m_type = FO4;
            }
            else if (magic == magic::DX10)
            {
                m_type = FO4dds;
            }
            else
            {
                throw runtime_error("Unknown FO4 archive subtype " + MagicToString(magic));
            }
            // seek to previous location
            seek(-4, CUR);

            break;
        }
        case headerVersions::SF: {
            // read ahead to get subtype, this is just to check for errors
            auto magic = read<uint32_t>();
            if (magic == magic::GNRL)
            {
                m_type = SF;
            }
            else
            {
                throw runtime_error("Unknown SF archive subtype " + MagicToString(magic));
            }
            // seek to previous location
            seek(-4, CUR);

            break;
        }

        case headerVersions::SFdds: {
            // read ahead to get subtype, this is just to check for errors
            auto magic = read<uint32_t>();
            if (magic == magic::DX10)
            {
                m_type = SFdds;
            }
            else
            {
                throw runtime_error("Unknown SF archive subtype " + MagicToString(magic));
            }
            // seek to previous location
            seek(-4, CUR);

            break;
        }
        [[unlikely]] default:
            throw runtime_error(format("Unknown archive version {:#x}", m_version));
    }
}

Bsa::Bsa(const std::filesystem::path &archivePath, bool multithreaded) noexcept(false)
    : m_existingArchive(true)
    , m_multithreaded(multithreaded)
    , m_md(EVP_MD_fetch(nullptr, "MD5", nullptr))
{
    m_file.reset(fopen(archivePath.string().c_str(), "rb"));

    if (m_file == nullptr)
    {
        const int error = errno;
        throw runtime_error(format("Could not open file \"{}\" for reading: {}", archivePath.string(), strerror(error)));
    }

    m_magic = read<uint32_t>();

    switch (m_magic)
    {
        case magic::TES3: m_type = TES3; break;
        case magic::BSA: m_type = TES4; break;
        case magic::BTDX: m_type = FO4; break;
        default: throw runtime_error("Unknown archive format, magic is "s + MagicToString(m_magic));
    }

    determineArchiveVersion();

    switch (m_type)
    {
        case TES3: readArchiveTes3(); break;

        case TES4:
        case FO3:
        case SSE: readArchiveTes4(); break;

        case FO4:
            m_header = read<HeaderFO4>();
            readBa2GNRL();
            readFO4FileTable();
            break;

        case FO4dds:
            m_header = read<HeaderFO4>();
            readBa2DX10();
            readFO4FileTable();
            break;

        case SF:
            m_header = read<HeaderSF>();
            readBa2GNRL();
            readFO4FileTable();
            break;

        case SFdds: {
            auto header = read<HeaderSFdds>();
            if (header.compressionMethod == 3)
            {
                m_compressionType = lz4Block;
            }
            m_header = header;

            readBa2DX10();
            readFO4FileTable();
            break;
        }

        case none:
            // don't do anything
            break;
    }
    m_fileName = archivePath;
}

void Bsa::createArchiveTES3(std::vector<std::filesystem::path> &fileList) noexcept(false)
{
    if (fileList.empty())
    {
        throw runtime_error("Archive requires predefined files list");
    }

    unordered_map<fs::path, uint64_t> hashMap;
    for (const auto &file : fileList)
    {
        hashMap.try_emplace(file, CreateHashTES3(file));
    }

    // sort by hash
    sort(std::execution::par_unseq, fileList.begin(), fileList.end(), [&hashMap](const fs::path &a, const fs::path &b) {
        return hashMap.at(a) < hashMap.at(b);
    });

    // create file records and calculate total names length
    m_files.reserve(fileList.size());
    size_t len = 0;
    for (const auto &file : fileList)
    {
        FileTES3 fileTES3;
        fileTES3.hash = hashMap.at(file);
        fileTES3.name = ToLower(file.string());
        len += fileTES3.name.string().length() + 1;
        m_files.emplace_back(fileTES3);

        addToFileMap(file, &get<FileTES3>(m_files.back()));
    }

    auto &headerTES3 = get<HeaderTES3>(m_header);

    // offset to hash tableDataOffset =
    m_dataOffset = sizeof(Magic4) + sizeof(HeaderTES3) + 8 * m_files.size() + // File sizes/offsets
                   4 * m_files.size() +                                       // Archive directory/name offsets
                   len;                                                       // Filename records

    // stored as minus 12 (for header size)
    headerTES3.hashOffset = gsl_lite::narrow<uint32_t>(m_dataOffset - sizeof(Magic4) - sizeof(HeaderTES3));
    headerTES3.fileCount = gsl_lite::narrow<uint32_t>(fileList.size());

    // offset to files data
    m_dataOffset = m_dataOffset + 8 * m_files.size(); // Hash table

    // files are stored alphabetically in the data section of vanilla archives
    sort(std::execution::par_unseq, fileList.begin(), fileList.end(), [](const fs::path &a, const fs::path &b) {
        return a.string() < b.string();
    });
}

void Bsa::createArchiveTES4(std::vector<std::filesystem::path> &fileList) noexcept(false)
{
    if (fileList.empty())
    {
        throw runtime_error("Archive requires predefined files list");
    }

    auto &headerTES4 = get<HeaderTES4>(m_header);
    headerTES4.folderNamesLength = 0;
    headerTES4.fileNamesLength = 0;

    // path, directory hash, file hash
    std::unordered_map<fs::path, pair<uint64_t, uint64_t>> hashMap;

    // directories and files must be sorted by their hashes
    for (const auto &file : fileList)
    {
        // calculate hashes
        if (!file.has_parent_path())
        {
            throw runtime_error("File is missing the folder part: "s + file.string());
        }
        fs::path dirPath = file.parent_path();
        fs::path filePath = file.filename();

        bool inserted = hashMap
                            .try_emplace(file, make_pair(CreateHashTES4(dirPath, true), CreateHashTES4(filePath, false)))
                            .second;

        if (!inserted)
        {
            throw runtime_error(format("Could not insert HashPair for file \"{}\" into hash map", file.string()));
        }

        string ext = ToLower(file.extension().string());
        string dir = ToLower(dirPath.generic_string());

        // determine file flags
        if (dir.starts_with("textures/") || ext == ".dds")
        {
            headerTES4.fileFlags |= flags::file::DDS;
        }
        else if (dir.starts_with("meshes/") || ext == ".nif" || ext == ".lod" || ext == ".bto" || ext == ".btr"
                 || ext == ".btt" || ext == ".dtl" || ext == ".kf" || ext == ".kfm" || ext == ".hkx")
        {
            headerTES4.fileFlags |= flags::file::NIF;
        }
        else if (dir.starts_with("sound/"))
        {
            headerTES4.fileFlags |= flags::file::WAV | flags::file::MP3;
        }
        else if (ext == ".xml")
        {
            headerTES4.fileFlags |= flags::file::XML | flags::file::MISC;
        }
        else if (ext == ".wav" || ext == ".fuz")
        {
            headerTES4.fileFlags |= flags::file::WAV;
        }
        else if (ext == ".lip" || ext == ".mp3" || ext == ".ogg")
        {
            headerTES4.fileFlags |= flags::file::MP3;
        }
        else if (ext == ".txt" || ext == ".htm" || ext == ".bat" || ext == ".scc")
        {
            headerTES4.fileFlags |= flags::file::TXT;
        }
        else if (ext == ".spt")
        {
            headerTES4.fileFlags |= flags::file::SPT;
        }
        else if (ext == ".fnt" || ext == ".tex")
        {
            headerTES4.fileFlags |= flags::file::FNT;
        }
        else
        {
            headerTES4.fileFlags |= flags::file::MISC;
        }

        // determine archive flags

        // packed scripts can't be added to objects in the SSE CK if the archive was packed
        // without the "RetainNames" flag (the scripts aren't shown in the script adding window)
        if (ext == ".pex")
        {
            headerTES4.flags |= flags::archive::RETAINNAME;
        }
    }

    // sort files by hash
    sort(std::execution::par_unseq, fileList.begin(), fileList.end(), [&hashMap](const fs::path &a, const fs::path &b) {
        const auto &hashA = hashMap.at(a);
        const auto &hashB = hashMap.at(b);
        if (hashA.first != hashB.first)
        {
            return hashA.first < hashB.first;
        }
        return hashA.second < hashB.second;
    });

    // create folder and file records
    headerTES4.fileCount = 0;
    uint64_t prevDirHash = 0;

    for (auto &file : fileList)
    {
        auto [dirHash, fileHash] = hashMap[file];
        // new folder
        if (dirHash != prevDirHash)
        {
            prevDirHash = dirHash;

            FolderTES4 folderTES4;
            folderTES4.hash = dirHash;
            folderTES4.name = ToLower(file.parent_path());

            m_files.emplace_back(folderTES4);

            // calculate folder names length
            headerTES4.folderNamesLength += gsl_lite::narrow<uint32_t>(
                folderTES4.name.string().length() + 1); // + terminator only, length prefix is not counted
        }
        FileTES4 fileTES4;
        fileTES4.hash = fileHash;
        fileTES4.name = ToLower(file.filename());

        auto &last = get<FolderTES4>(m_files.back());
        last.files.emplace_back(fileTES4);
        last.fileCount++;

        headerTES4.fileCount++;
        // calculate file names length
        // NOTE: u16string is required to get the correct length for non-ASCII file names
        // when using string, "dlc2mq05__0003c745_1 .fuz" length would be 26 instead of 25
        headerTES4.fileNamesLength += gsl_lite::narrow<uint32_t>(fileTES4.name.u16string().length() + 1);
    }
    headerTES4.folderCount = gsl_lite::narrow<uint32_t>(m_files.size());

    // calculate folders offsets
    // at the end fDataOffset will hold the total size of header, folder and file records
    // in other words the start of files data
    m_dataOffset = sizeof(Magic4) + sizeof(m_version) + sizeof(HeaderTES4) + 16 * m_files.size();
    // SSE folder record is 8 bytes larger
    if (m_type == SSE)
    {
        m_dataOffset += 8 * m_files.size();
    }
    // offsets are stored including this value
    m_dataOffset += headerTES4.fileNamesLength;

    for (auto &_folder : m_files)
    {
        auto &folder = get<FolderTES4>(_folder);
        folder.offset = m_dataOffset;
        // add folder name length
        m_dataOffset += folder.name.string().size() + 2; // + length prefix + terminator
        // add file records length
        m_dataOffset += 16 * folder.files.size();
    }

    // final flags detection

    // misc file flag is not in Skyrim SE
    if (m_type == SSE)
    {
        headerTES4.fileFlags &= ~flags::file::MISC;
    }
    // embedded names in texture only archives
    // except Skyrim SE: crashing engine bug if texture is uncompressed and file name is embedded
    if (headerTES4.fileFlags == flags::file::DDS && m_type != SSE)
    {
        headerTES4.flags |= flags::archive::EMBEDNAME;
    }
    // startupstr flag in archives with meshes
    if ((headerTES4.fileFlags & flags::file::NIF) != 0)
    {
        headerTES4.flags |= flags::archive::STARTUPSTR;
    }
    // retain name flag in archives with sounds
    if ((headerTES4.fileFlags & flags::file::WAV) != 0)
    {
        headerTES4.flags |= flags::archive::RETAINNAME;
    }
    // txt, xml and fnt file flags are exclusive for Oblivion
    if (m_type != TES4)
    {
        headerTES4.fileFlags &= ~(flags::file::XML | flags::file::TXT | flags::file::FNT);
    }
    // set compression flag if needed
    if (m_compressed)
    {
        headerTES4.flags |= flags::archive::COMPRESS;
    }
}

void Bsa::createArchiveFO4(std::vector<std::filesystem::path> &fileList) noexcept(false)
{
    if (fileList.empty())
    {
        throw runtime_error("Archive requires predefined files list");
    }
    try
    {
        // sort files alphabetically
        sort(std::execution::par_unseq, fileList.begin(), fileList.end(), comparePaths);

        getHeaderFO4().fileCount = gsl_lite::narrow<uint32_t>(fileList.size());
        m_files.reserve(fileList.size());
        for (const auto &file : fileList)
        {
            if (!file.has_parent_path())
            {
                throw runtime_error("File is missing the folder part: "s + file.string());
            }

            fs::path name = file.filename().stem();
            std::string ext = file.extension().string();
            // remove leading '.'
            if (ext[0] == '.')
            {
                ext.erase(0, 1);
            }

            FileFO4 fileFO4;
            fileFO4.name = file.string();
            fileFO4.dirHash = CreateHashFO4(file.parent_path());
            fileFO4.nameHash = CreateHashFO4(name);
            fileFO4.ext = StringToMagic(ToLower(ext));
            fileFO4.unknown = iFileFO4Unknown;
            m_files.emplace_back(fileFO4);

            addToFileMap(file, &get<FileFO4>(m_files.back()));
        }

        m_dataOffset = sizeof(Magic4) + sizeof(m_version);

        switch (m_type)
        {
            case FO4:
            case FO4dds: m_dataOffset += sizeof(HeaderFO4); break;
            case SF: m_dataOffset += sizeof(HeaderSF); break;
            case SFdds: m_dataOffset += sizeof(HeaderSFdds); break;
            default:
                throw runtime_error(format("{} was called with wrong archive type {}", __FUNCTION__, (int) m_type));
        }

        // file records have fixed length in general archive
        if (m_type == FO4 || m_type == SF)
        {
            m_dataOffset += sizes::fileRecordGNRL * m_files.size();
        }

        // variable file record length depending on DDS chunks number
        else if (m_type == FO4dds || m_type == SFdds)
        {
            if (m_ddsBasePath.empty())
            {
                throw runtime_error("DDS Archive requires setting a DDS Base Path");
            }

            for (auto &file : fileList)
            {
                DDSInfo ddsInfo = getDDSInfo(file);
                m_dataOffset += sizes::fileRecordDDS + sizes::texChunk * getDDSMipChunkCount(ddsInfo);
            }
        }
    }
    catch (...)
    {
        throw;
    }
}

Bsa::Bsa(const std::filesystem::path &archivePath,
         ArchiveType type,
         std::vector<std::filesystem::path> &fileList,
         const std::optional<std::filesystem::path> &ddsBasePath,
         bool compressed,
         bool shareData,
         bool multithreaded) noexcept(false)
    : m_existingArchive(false)
    , m_type(type)
    , m_ddsBasePath(ddsBasePath.value_or(std::filesystem::path()))
    , m_compressed(compressed)
    , m_shareData(shareData)
    , m_multithreaded(multithreaded)
    , m_md(EVP_MD_fetch(nullptr, "MD5", nullptr))
{
    try
    {
        switch (type)
        {
            case TES3:
                m_magic = magic::TES3;
                m_header = HeaderTES3{};

                createArchiveTES3(fileList);
                break;
            case TES4: {
                m_version = headerVersions::TES4;
                m_magic = magic::BSA;
                m_header = HeaderTES4{};
                auto &headerTES4 = get<HeaderTES4>(m_header);
                headerTES4.flags = flags::archive::PATHNAMES | flags::archive::FILENAMES | flags::archive::EMBEDNAME
                                   | flags::archive::XMEM | flags::archive::UNKNOWN10;
                headerTES4.fileFlags = 0;
                headerTES4.foldersOffset = sizeof(Magic4) + sizeof(m_version) + sizeof(HeaderTES4);
                m_compressionType = zlib;

                createArchiveTES4(fileList);
                break;
            }
            case FO3: {
                m_version = headerVersions::FO3;
                m_magic = magic::BSA;
                m_header = HeaderTES4{};
                auto &headerTES4 = get<HeaderTES4>(m_header);
                headerTES4.flags = flags::archive::PATHNAMES | flags::archive::FILENAMES;
                headerTES4.fileFlags = 0;
                headerTES4.foldersOffset = sizeof(Magic4) + sizeof(m_version) + sizeof(HeaderTES4);
                m_compressionType = zlib;

                createArchiveTES4(fileList);
                break;
            }
            case SSE: {
                m_version = headerVersions::SSE;
                m_magic = magic::BSA;
                m_header = HeaderTES4{};
                auto &headerTES4 = get<HeaderTES4>(m_header);
                headerTES4.flags = flags::archive::PATHNAMES | flags::archive::FILENAMES;
                headerTES4.fileFlags = 0;
                headerTES4.foldersOffset = sizeof(Magic4) + sizeof(m_version) + sizeof(HeaderTES4);
                m_compressionType = lz4Frame;

                createArchiveTES4(fileList);
                break;
            }
            case FO4: {
                m_magic = magic::BTDX;
                HeaderFO4 header;
                header.magic = magic::GNRL;
                m_header = header;
                m_version = headerVersions::FO4v1;
                m_compressionType = zlib;

                createArchiveFO4(fileList);
                break;
            }
            case FO4dds: {
                m_magic = magic::BTDX;
                HeaderFO4 header;
                header.magic = magic::DX10;
                m_header = header;
                m_version = headerVersions::FO4v1;
                m_compressionType = zlib;

                createArchiveFO4(fileList);
                break;
            }
            case SF: {
                m_magic = magic::BTDX;
                HeaderSF header;
                header.fo4Header.magic = magic::GNRL;
                m_header = header;
                m_version = headerVersions::SF;
                m_compressionType = zlib;

                createArchiveFO4(fileList);
                break;
            }
            case SFdds: {
                m_magic = magic::BTDX;
                HeaderSFdds header;
                header.fo4Header.magic = magic::DX10;
                m_header = header;
                m_version = headerVersions::SFdds;
                m_compressionType = lz4Block;

                createArchiveFO4(fileList);
                break;
            }
            [[unlikely]] default:
                throw runtime_error("Unsupported archive type");
        }

        create_directories(archivePath.parent_path());
        m_file.reset(fopen(archivePath.string().c_str(), "wb"));
        if (m_file == nullptr)
        {
            const int error = errno;
            throw runtime_error(
                format("Error opening file \"{}\" for writing: {}", archivePath.string(), strerror(error)));
        }

        m_fileName = archivePath;

        // reserve space for the header
        Buffer buffer(m_dataOffset, 0);
        write(buffer);
    }
    catch (...)
    {
        throw;
    }
}

Bsa::Bsa(const filesystem::path &archivePath,
         ArchiveType type,
         std::vector<std::filesystem::path> &fileList,
         const std::optional<std::filesystem::path> &ddsBasePath) noexcept(false)
    : m_existingArchive(false)
{
    try
    {
        Bsa(archivePath, type, fileList, ddsBasePath, false, false, false);
    }
    catch (...)
    {
        throw;
    }
}

Bsa::~Bsa()
{
    EVP_MD_free(m_md);
}

std::string Bsa::getArchiveFormatName() const noexcept
{
    return ToString(m_type);
}

FileRecord_t Bsa::findFileRecordTES4(const std::filesystem::path &filePath) noexcept
{
    fs::path dir;
    if (!filePath.has_extension())
    {
        // treat as directory
        dir = filePath;
    }
    else
    {
        dir = filePath.has_parent_path() ? filePath.parent_path() : "";
    }

    const uint64_t dirHash = CreateHashTES4(dir, true);
    for (auto &_folder : m_files)
    {
        auto &folder = get<FolderTES4>(_folder);
        // since table is sorted by hash, we can abort when our hash is lesser
        if (dirHash < folder.hash)
        {
            return nullptr;
        }

        if (dirHash == folder.hash)
        {
            const uint64_t fileHash = CreateHashTES4(filePath.filename(), false);
            for (uint32_t j = 0; j < folder.fileCount; j++)
            {
                // since table is sorted by hash, we can abort when our hash is lesser
                if (fileHash < folder.files[j].hash)
                {
                    return nullptr;
                }
                if (fileHash == folder.files[j].hash)
                {
                    return &folder.files[j];
                }
            }
        }
    }
    return nullptr;
}

void Bsa::addToFileMap(const std::filesystem::path &filePath, FilePtr_t file) noexcept(false)
{
    lock_guard guard(m_fileMapMtx);
    if (!m_fileMap.try_emplace(filePath, file).second)
    {
        throw runtime_error(format("Could not add \"{}\" to file map", filePath.string()));
    }
}

int Bsa::getDDSMipChunkCount(const DDSInfo &DDSInfo) const noexcept
{
    int count = 1;
    int width = DDSInfo.width;
    int height = DDSInfo.height;

    while (count < DDSInfo.mipMaps && count < m_maxChunkCount && width >= m_singleMipChunkX
           && height >= m_singleMipChunkY)
    {
        count++;
        width /= 2;
        height /= 2;
    }
    return count;
}

PackedDataHash Bsa::calcDataHash(const uint8_t *data, const uint32_t length) const noexcept
{
    // calculate md5
    EVP_MD_CTX *context = EVP_MD_CTX_new();
    PackedDataHash md_value{};
    unsigned int md_len = 0;

    EVP_DigestInit_ex2(context, m_md, nullptr);
    EVP_DigestUpdate(context, data, length);
    EVP_DigestFinal_ex(context, md_value.data(), &md_len);
    EVP_MD_CTX_free(context);

    return md_value;
}

bool Bsa::findPackedData(const uint32_t size, const PackedDataHash &hash, const FileRecord_t &fileRecord) noexcept
{
    if (!m_shareData)
    {
        return false;
    }

    unique_lock l(m_packedDataMtx, defer_lock);
    if (m_multithreaded) // only lock if actually using multithreading
    {
        l.lock();
    }

    for (const auto &packedData : m_packedData)
    {
        // check if sizes and hashes are identical
        if (size == packedData.size && memcmp(hash.data(), packedData.hash.data(), sizeof(PackedDataHash)) == 0)
        {
            switch (m_type)
            {
                case TES3: {
                    auto *const a = get<FileTES3 *>(fileRecord);
                    const auto *b = get<FileTES3 *>(packedData.fileRecord);

                    a->size = b->size;
                    a->offset = b->offset;

                    break;
                }
                case TES4:
                case FO3:
                case SSE: {
                    auto *const a = get<FileTES4 *>(fileRecord);
                    const auto *b = get<FileTES4 *>(packedData.fileRecord);

                    a->size = b->size;
                    a->offset = b->offset;

                    break;
                }
                case FO4:
                case SF: {
                    auto *const a = get<FileFO4 *>(fileRecord);
                    const auto *b = get<FileFO4 *>(packedData.fileRecord);

                    a->size = b->size;
                    a->packedSize = b->packedSize;
                    a->offset = b->offset;

                    break;
                }
                case FO4dds:
                case SFdds: {
                    auto *const a = get<TexChunkRec *>(fileRecord);
                    const auto *b = get<TexChunkRec *>(packedData.fileRecord);

                    a->size = b->size;
                    a->packedSize = b->packedSize;
                    a->offset = b->offset;

                    break;
                }
                [[unlikely]] default:
                    return false;
            }
            return true;
        }
    }
    return false;
}

int64_t Bsa::getCreatedArchiveSize() const noexcept
{
    if (!m_existingArchive && m_file != nullptr)
    {
        return _ftelli64(m_file.get());
    }
    return 0;
}

uint32_t Bsa::getArchiveFlags() const noexcept
{
    if (holds_alternative<HeaderTES4>(m_header))
    {
        return get<HeaderTES4>(m_header).flags;
    }
    return 0;
}

void Bsa::addFile(const filesystem::path &rootDirectory, const filesystem::path &filePath) noexcept(false)
{
    if (m_abort.load())
    {
        return;
    }
    if (m_existingArchive)
    {
        throw runtime_error("Cannot add a file to an existing archive");
    }
    if (!exists(filePath))
    {
        throw runtime_error(format("File \"{}\" does not exist", filePath.generic_string()));
    }

    // strip rootDirectory from filePath
    // this is significantly faster than using filesystem::relative
    size_t pos = rootDirectory.generic_string().length();
    // increment pos if rootDirectory does not end with '/'
    if (pos > 1 && !rootDirectory.generic_string().ends_with('/'))
    {
        pos++;
    }
    const fs::path targetPath = filePath.generic_string().substr(pos);

    const filePtr inputFile(fopen(filePath.generic_string().c_str(), "rb"));

    if (inputFile.get() == nullptr)
    {
        const int error = errno;
        throw runtime_error(
            format("Could not open  \"{}\" for reading: {}", filePath.generic_string(), strerror(error)));
    }

    // get file size
    const size_t size = file_size(filePath);

    Buffer buffer(size);
    if (fread(buffer.data(), 1, size, inputFile.get()) != size)
    {
        const int error = errno;
        throw runtime_error("Read error: "s + strerror(error));
    }

    addFile(targetPath, buffer);
}

void Bsa::addFileDDS(FileFO4 *file, const Buffer &data) noexcept(false)
{
    try
    {
        file->unknownTex = 0;
        // DDS file parameters
        const auto *const ddsHeader = reinterpret_cast<const DDSHeader *>(data.data());
        uint32_t offset = sizeof(DDSHeader); // offset to image data
        file->width = gsl_lite::narrow<uint16_t>(ddsHeader->width);
        file->height = gsl_lite::narrow<uint16_t>(ddsHeader->height);
        file->numMips = gsl_lite::narrow<uint8_t>(ddsHeader->mipMapCount);
        // no mipmaps is equal to a single one
        if (file->numMips == 0)
        {
            file->numMips = 1;
        }

        if (ddsHeader->ddspf.fourCC == magic::DX10)
        {
            offset += sizeof(DDSHeaderDX10);
        }

        // DXGI detection
        file->dxgiFormat = getDxgiFormat(data);

        // MipMap size detection
        int bpp = bitsPerPixel(file->dxgiFormat);

        uint32_t MipSize = file->width * file->height * bpp >> 3;

        // cube maps detection
        file->cubeMaps = 0x800;
        if ((ddsHeader->caps2 & DDSCAPS2_CUBEMAP) != 0U)
        {
            file->cubeMaps |= 1;
        }
        DDSInfo ddsInfo;

        // number of chunks to store in file record
        ddsInfo.width = file->width;
        ddsInfo.height = file->height;
        ddsInfo.mipMaps = file->numMips;

        const auto count = gsl_lite::narrow<uint16_t>(getDDSMipChunkCount(ddsInfo));

        file->texChunks.reserve(count);

        // storing chunks
        for (uint16_t x = 0; x < count; x++)
        {
            TexChunkRec texChunk;
            texChunk.startMip = x;

            if (x < count - 1)
            {
                texChunk.endMip = x;
            }
            else
            {
                // last chunk stores all remaining mipmaps
                texChunk.endMip = file->numMips - 1;
                MipSize = data.size() - offset;
            }

            PackedDataHash dataHash;
            if (m_shareData)
            {
                // only calculate hash when needed
                dataHash = calcDataHash(&data[offset], MipSize);
            }

            file->texChunks.emplace_back(texChunk);

            // force compression
            packData(&file->texChunks[x],
                     file->name,
                     dataHash,
                     &data[offset],
                     MipSize,
                     file->compress(this),
                     file->compress(this));
            offset += MipSize;
            MipSize /= 4;
        }
    }
    catch (const gsl_lite::narrowing_error &ex)
    {
        throw runtime_error(ex.what());
    }
    catch (const runtime_error &)
    {
        throw;
    }
}

int Bsa::bitsPerPixel(uint8_t format) noexcept(false)
{
    switch (format)
    {
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM: return 4;
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
        case DXGI_FORMAT_A8_UNORM:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_UNORM: return 8;
        case DXGI_FORMAT_B5G6R5_UNORM:
        case DXGI_FORMAT_B5G5R5A1_UNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_UNORM: return 16;
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return 32;

        default: throw runtime_error("Unsupported DDS format");
    }
}

DXGI_FORMAT Bsa::getDxgiFormat(const Buffer &data) noexcept(false)
{
    const auto *ddsHeader = reinterpret_cast<const DDSHeader *>(data.data());

    switch (ddsHeader->ddspf.fourCC)
    {
        case magic::DXT1: return DXGI_FORMAT_BC1_UNORM; break;
        case magic::DXT3: return DXGI_FORMAT_BC2_UNORM; break;
        case magic::DXT5: return DXGI_FORMAT_BC3_UNORM; break;
        case magic::ATI1:
        case magic::BC4U: return DXGI_FORMAT_BC4_UNORM; break;
        case magic::BC4S: return DXGI_FORMAT_BC4_SNORM; break;
        case magic::ATI2:
        case magic::BC5U: return DXGI_FORMAT_BC5_UNORM; break;
        case magic::BC5S: return DXGI_FORMAT_BC5_SNORM; break;
        case magic::DX10: {
            const auto *ddsHeaderDX10 = reinterpret_cast<const DDSHeaderDX10 *>(&data[sizeof(DDSHeader)]);
            return static_cast<DXGI_FORMAT>(ddsHeaderDX10->dxgiFormat);
            break;
        }
        default:
            switch (ddsHeader->ddspf.RGBBitCount)
            {
                case 32:
                    if ((ddsHeader->ddspf.flags & DDPF_ALPHAPIXELS) == 0)
                    {
                        return DXGI_FORMAT_B8G8R8X8_UNORM;
                    }
                    if (ddsHeader->ddspf.RBitMask == 0x000000FF)
                    {
                        return DXGI_FORMAT_R8G8B8A8_UNORM;
                    }
                    return DXGI_FORMAT_B8G8R8A8_UNORM;

                case 16:
                    if (ddsHeader->ddspf.RBitMask == 0xF800 && ddsHeader->ddspf.GBitMask == 0x07E0
                        && ddsHeader->ddspf.BBitMask == 0x001F && ddsHeader->ddspf.ABitMask == 0x0000)
                    {
                        return DXGI_FORMAT_B5G6R5_UNORM;
                    }
                    if (ddsHeader->ddspf.RBitMask == 0x7C00 && ddsHeader->ddspf.GBitMask == 0x03E0
                        && ddsHeader->ddspf.BBitMask == 0x001F && ddsHeader->ddspf.ABitMask == 0x8000)
                    {
                        return DXGI_FORMAT_B5G5R5A1_UNORM;
                    }
                    return DXGI_FORMAT_R8G8_UNORM;

                case 8:
                    if ((ddsHeader->ddspf.flags & DDPF_ALPHA) != 0)
                    {
                        return DXGI_FORMAT_A8_UNORM;
                    }
                    return DXGI_FORMAT_R8_UNORM;
                default: throw runtime_error("Unsupported uncompressed DDS format");
            }
    }
}

void Bsa::addFile(const std::filesystem::path &filePath, Buffer &data) noexcept(false)
{
    if (m_abort.load())
    {
        return;
    }
    PackedDataHash dataHash;

    if (m_existingArchive)
    {
        throw runtime_error("Archive is not in writing mode");
    }

    // dds mipmaps have their own partial hash calculation down below
    if (m_shareData && m_type != FO4dds && m_type != SFdds)
    {
        dataHash = calcDataHash(data.data(), data.size());
    }

    unique_lock lock(m_writeMtx, defer_lock);
    if (m_multithreaded)
    {
        lock.lock();
    }

    try
    {
        switch (m_type)
        {
            case TES3: {
                auto file = findFileRecord(filePath);
                if (holds_alternative<nullptr_t>(file))
                {
                    throw runtime_error(format("File \"{}\" not found in files table", filePath.string()));
                }
                packData(file, filePath, dataHash, data.data(), data.size(), false);
                break;
            }
            case TES4:
            case FO3:
            case SSE: {
                auto file = findFileRecord(filePath);
                if (holds_alternative<nullptr_t>(file))
                {
                    throw runtime_error(format("File \"{}\" not found in files table", filePath.string()));
                }
                packData(file, filePath, dataHash, data.data(), data.size(), get<FileTES4 *>(file)->compress(this));
                break;
            }
            case FO4:
            case SF: {
                auto file = findFileRecord(filePath);
                if (holds_alternative<nullptr_t>(file))
                {
                    throw runtime_error(format("File \"{}\" not found in files table", filePath.string()));
                }

                auto *fileFO4 = get<FileFO4 *>(file);

                fileFO4->offset = _ftelli64(m_file.get());
                fileFO4->size = data.size();

                packData(fileFO4, fileFO4->name, dataHash, data.data(), data.size(), fileFO4->compress(this));
                break;
            }

            case FO4dds:
            case SFdds: {
                auto file = findFileRecord(filePath);
                if (holds_alternative<nullptr_t>(file))
                {
                    throw runtime_error(format("File \"{}\" not found in files table", filePath.string()));
                }

                auto *fileFO4 = get<FileFO4 *>(file);

                try
                {
                    addFileDDS(fileFO4, data);
                }
                catch (...)
                {
                    throw;
                }
                break;
            }
            [[unlikely]] case none:
                throw runtime_error("Archive type cannot be None when adding files");
        }
    }
    catch (...)
    {
        throw;
    }
}

FileRecord_t Bsa::findFileRecord(const std::filesystem::path &fileName) noexcept
{
    try
    {
        switch (m_type)
        {
            case TES3: return get<FileTES3 *>(m_fileMap.at(fileName));
            case TES4:
            case FO3:
            case SSE: return findFileRecordTES4(fileName);
            case FO4:
            case FO4dds:
            case SF:
            case SFdds:
                return get<FileFO4 *>(m_fileMap.at(fileName));
            [[unlikely]] default:
                return nullptr;
        }
    }
    catch (const out_of_range &)
    {
        return nullptr;
    }
}

Buffer Bsa::decompressData(const Buffer &data, const uint32_t uncompressedSize) const noexcept(false)
{
    Buffer result(uncompressedSize, 0);
    decompressData(data, result.data(), uncompressedSize);
    return result;
}

void Bsa::decompressData(const Buffer &compressed, uint8_t *uncompressed, uint32_t uncompressedSize) const
    noexcept(false)
{
    switch (m_compressionType)
    {
        case zlib: {
            uint32_t dstSize = uncompressedSize;

            uncompress(uncompressed,
                       reinterpret_cast<uLongf *>(&dstSize),
                       compressed.data(),
                       gsl_lite::narrow<uLong>(compressed.size()));

            if (dstSize != uncompressedSize)
            {
                throw runtime_error(format("Decompression error: dstSize has unexpected size, expected {}, got {}",
                                           uncompressedSize,
                                           dstSize));
            }

            break;
        }
        case lz4Frame: {
            // some code taken from https://github.com/lz4/lz4/blob/dev/examples/frameCompress.c
            LZ4F_dctx *dctxPtr = nullptr;
            const LZ4F_errorCode_t dctxStatus = LZ4F_createDecompressionContext(&dctxPtr, LZ4F_VERSION);

            if (LZ4F_isError(dctxStatus) != 0U)
            {
                throw runtime_error("LZ4F_dctx creation error: "s + LZ4F_getErrorName(dctxStatus));
            }

            // use unique_ptr to automatically call LZ4F_freeDecompressionContext when leaving scope
            const unique_ptr<LZ4F_dctx, decltype(&LZ4F_freeDecompressionContext)> dctx{dctxPtr,
                                                                                       LZ4F_freeDecompressionContext};

            const void *srcPtr = compressed.data();
            const void *const srcEnd = to_address(compressed.end());
            size_t result = 1;

            /* Decompress:
             * Continue while there is more input to read (srcPtr != srcEnd)
             * and the frame isn't over (result != 0)
             */
            while (srcPtr < srcEnd && result != 0)
            {
                size_t dstSize = uncompressedSize;
                size_t srcSize = (const char *) srcEnd - (const char *) srcPtr;
                result = LZ4F_decompress(dctx.get(), uncompressed, &dstSize, srcPtr, &srcSize, nullptr);
                /* Update input */
                srcPtr = (const char *) srcPtr + srcSize;
            }

            if (LZ4F_isError(result) != 0U)
            {
                throw runtime_error("Decompression error: "s + LZ4F_getErrorName(result));
            }

            if (srcPtr > srcEnd)
            {
                throw runtime_error("Decompress: srcPtr > srcEnd");
            }

            if (srcPtr < srcEnd)
            {
                throw runtime_error("Decompress: Trailing data left in file after frame");
            }

            break;
        }
        case lz4Block:
            LZ4_decompress_safe(reinterpret_cast<const char *>(compressed.data()),
                                reinterpret_cast<char *>(uncompressed),
                                gsl_lite::narrow<int>(compressed.size()),
                                gsl_lite::narrow<int>(uncompressedSize));

            break;
    }
}

Buffer Bsa::extractFileData(const std::filesystem::path &fileName) noexcept(false)
{
    if (!m_existingArchive)
    {
        throw runtime_error("Archive is not loaded");
    }

    const FileRecord_t fileRecord = findFileRecord(fileName);

    if (holds_alternative<nullptr_t>(fileRecord))
    {
        throw runtime_error("File not found in archive");
    }

    return extractFileData(fileRecord);
}

void Bsa::extractFile(const std::filesystem::path &filePath, const std::filesystem::path &saveAs) noexcept(false)
{
    if (m_abort.load())
    {
        return;
    }
    if (!m_existingArchive)
    {
        throw runtime_error("Archive is not loaded");
    }
    if (is_directory(saveAs))
    {
        throw runtime_error(format("Output file \"{}\" is a directory", saveAs.string()));
    }

    const FileRecord_t fileRecord = findFileRecord(filePath);

    if (holds_alternative<nullptr_t>(fileRecord))
    {
        throw runtime_error(format("File \"{}\" not found in archive", filePath.string()));
    }

    create_directories(saveAs.parent_path());
    const filePtr outputFile(fopen(saveAs.string().c_str(), "wb"));

    if (outputFile.get() == nullptr)
    {
        const int error = errno;
        throw runtime_error(format("Error opening file \"{}\" for writing: {}", saveAs.string(), strerror(error)));
    }

    const auto fileData = extractFileData(fileRecord);

    if (fwrite(fileData.data(), 1, fileData.size(), outputFile.get()) != fileData.size())
    {
        const int error = errno;
        throw runtime_error(format("Could not write to file \"{}\": {}", saveAs.string(), strerror(error)));
    }
}

void Bsa::iterateFiles(const FileIterationFunction &function, void *data) noexcept
{
    if (function == nullptr)
    {
        return;
    }

    switch (m_type)
    {
        case TES3:
            for (auto &file : m_files)
            {
                if (function(get<FileTES3>(file).name, &get<FileTES3>(file), nullptr, data))
                {
                    break;
                }
            }
            break;
        case TES4:
        case FO3:
        case SSE:
            for (auto &_folder : m_files)
            {
                auto &folder = get<FolderTES4>(_folder);
                for (auto &file : folder.files)
                {
                    if (function(folder.name / file.name, &file, &folder, data))
                    {
                        break;
                    }
                }
            }

            break;
        case FO4:
        case FO4dds:
        case SF:
        case SFdds:
            for (auto &file : m_files)
            {
                if (function(get<FileFO4>(file).name, &get<FileFO4>(file), nullptr, data))
                {
                    break;
                }
            }

            break;
        [[unlikely]] default:;
    }
}

std::filesystem::path Bsa::getFileName() const noexcept
{
    return m_fileName;
}

ArchiveType Bsa::getArchiveType() const noexcept
{
    return m_type;
}

uint32_t Bsa::getVersion() const noexcept
{
    return m_version;
}

bool Bsa::fileExists(const filesystem::path &filePath) noexcept
{
    return !holds_alternative<nullptr_t>(findFileRecord(filePath));
}

void Bsa::save() noexcept(false)
{
    if (m_existingArchive)
    {
        throw runtime_error("Archive is not in writing mode");
    }
    if (m_file == nullptr)
    {
        throw runtime_error("Archive file is not open");
    }

    unique_lock lock(m_writeMtx, defer_lock);
    if (m_multithreaded)
    {
        // only lock when multithreaded
        lock.lock();
    }

    switch (m_type)
    {
        case TES3: {
            // check that all files from files table have saved data
            for (const auto &_file : m_files)
            {
                const auto &file = get<FileTES3>(_file);
                if (file.offset == 0)
                {
                    throw runtime_error("Archived file has no data: "s + file.name.string());
                }
            }

            // write header
            seek(0);
            // magic, header record
            write(m_magic);
            write(get<HeaderTES3>(m_header));
            // file sizes/offsets
            for (const auto &_file : m_files)
            {
                const auto &file = get<FileTES3>(_file);
                write(file.size);
                write(file.offset - gsl_lite::narrow<uint32_t>(m_dataOffset)); // offsets are relative
            }
            // Archive directory/name offsets
            uint32_t j = 0;
            for (const auto &file : m_files)
            {
                write(j);
                j += gsl_lite::narrow<uint32_t>(get<FileTES3>(file).name.string().length()) + 1; // including terminator
            }
            // Filename records
            for (const auto &file : m_files)
            {
                write(get<FileTES3>(file).name);
            }
            // Hash table
            for (const auto &_file : m_files)
            {
                const auto &file = get<FileTES3>(_file);

                write(static_cast<uint32_t>(file.hash >> 32));
                write(static_cast<uint32_t>(file.hash & UINT32_MAX));
            }
            break;
        }
        case TES4:
        case FO3:
        case SSE: {
            // check that all files from files table have saved data
            for (const auto &_folder : m_files)
            {
                const auto &folder = get<FolderTES4>(_folder);
                for (const auto &file : folder.files)
                {
                    if (file.offset == 0)
                    {
                        throw runtime_error("Archived file has no data: "s + (folder.name / file.name).string());
                    }
                }
            }
            // write header
            seek(0);
            // magic, version, header record
            write(m_magic);
            write(m_version);
            write(get<HeaderTES4>(m_header));
            // folder records
            for (const auto &_folder : m_files)
            {
                const auto &folder = get<FolderTES4>(_folder);
                write(folder.hash);
                write(folder.fileCount);
                if (m_type == SSE)
                {
                    write(folder.unk32);
                    write(folder.offset);
                }
                else
                {
                    write(gsl_lite::narrow<uint32_t>(folder.offset));
                }
            }
            // folder names and file records
            for (const auto &_folder : m_files)
            {
                const auto &folder = get<FolderTES4>(_folder);
                writeStringLen8(folder.name);
                for (const auto &file : folder.files)
                {
                    write(file.hash);
                    write(file.size);
                    write(file.offset);
                }
            }
            // file names
            for (const auto &_folder : m_files)
            {
                const auto &folder = get<FolderTES4>(_folder);
                for (const auto &file : folder.files)
                {
                    write(file.name);
                }
            }
            break;
        }
        case FO4:
        case SF: {
            // check that all files from files table have saved data
            for (const auto &_file : m_files)
            {
                const auto &file = get<FileFO4>(_file);

                if (file.offset == 0)
                {
                    throw runtime_error("Archived file has no data: "s + file.name.string());
                }
            }
            // file names table
            getHeaderFO4().fileTableOffset = _ftelli64(m_file.get());
            for (const auto &file : m_files)
            {
                writeStringLen16(get<FileFO4>(file).name);
            }
            // write header
            seek(0);
            // magic, version, header record
            write(m_magic);
            write(m_version);
            if (holds_alternative<HeaderFO4>(m_header))
            {
                write(get<HeaderFO4>(m_header));
            }
            else if (holds_alternative<HeaderSF>(m_header))
            {
                auto &headerSF = get<HeaderSF>(m_header);
                headerSF.unknown1 = 1;
                headerSF.unknown2 = 0;
                write(headerSF);
            }
            else
            {
                throw runtime_error("Invalid header type");
            }

            // file records
            for (const auto &_file : m_files)
            {
                const auto &file = get<FileFO4>(_file);
                write(file.nameHash);
                write(file.ext);
                write(file.dirHash);
                write(file.unknown);
                write(file.offset);
                write(file.packedSize);
                write(file.size);
                write(FileFO4Footer);
            }
            break;
        }

        case FO4dds:
        case SFdds: {
            // check that all files from files table have saved data
            for (const auto &_file : m_files)
            {
                const auto &file = get<FileFO4>(_file);
                if (file.texChunks.empty())
                {
                    throw runtime_error("Archived file has no data: "s + file.name.string());
                }
            }

            // file names table
            getHeaderFO4().fileTableOffset = _ftelli64(m_file.get());
            for (const auto &file : m_files)
            {
                writeStringLen16(get<FileFO4>(file).name);
            }
            // write header
            seek(0);
            // magic, version, header record
            write(m_magic);
            write(m_version);
            if (holds_alternative<HeaderFO4>(m_header))
            {
                write(get<HeaderFO4>(m_header));
            }
            else
            {
                auto &header = get<HeaderSFdds>(m_header);
                header.unknown1 = 1;
                header.unknown2 = 0;
                header.compressionMethod = 3; // lz4
                write(header);
            }

            // file records
            for (const auto &_file : m_files)
            {
                const auto &file = get<FileFO4>(_file);
                write(file.nameHash);
                write(file.ext);
                write(file.dirHash);
                write(file.unknownTex);
                write(gsl_lite::narrow<uint8_t>(file.texChunks.size()));
                write(sizes::chunkHeader);
                write(file.height);
                write(file.width);
                write(file.numMips);
                write(file.dxgiFormat);
                write(file.cubeMaps);
                for (const auto &chunk : file.texChunks)
                {
                    write(static_cast<uint64_t>(chunk.offset));
                    write(chunk.packedSize);
                    write(chunk.size);
                    write(chunk.startMip);
                    write(chunk.endMip);
                    write(FileFO4Footer);
                }
            }
            break;
        }
        default: throw runtime_error("Invalid archive type for saving");
    }
}

void Bsa::addPackedData(const uint32_t size, const PackedDataHash &hash, const FileRecord_t &fileRecord) noexcept
{
    if (!m_shareData)
    {
        return;
    }

    unique_lock lock(m_packedDataMtx, defer_lock);
    if (m_multithreaded)
    {
        lock.lock();
    }

    // reserve memory more elements
    if (m_packedData.size() == m_packedData.capacity())
    {
        m_packedData.reserve(m_packedData.size() + sizes::packedDataReserveCount);
    }

    PackedDataInfo info;

    info.size = size;
    info.hash = hash;
    info.fileRecord = fileRecord;
    m_packedData.emplace_back(info);
}

Buffer Bsa::extractFileData(const FileRecord_t &fileRecord) noexcept(false)
{
    if (!m_existingArchive)
    {
        throw runtime_error("Archive is not loaded");
    }
    if (holds_alternative<nullptr_t>(fileRecord))
    {
        throw runtime_error("File Record is nullptr");
    }

    Buffer retVal;

    unique_lock lock(m_writeMtx, defer_lock);
    if (m_multithreaded)
    {
        lock.lock();
    }

    switch (m_type)
    {
        case TES3: {
            const auto *fileTES3 = get<FileTES3 *>(fileRecord);
            seek(gsl_lite::narrow<int64_t>(m_dataOffset + fileTES3->offset));
            retVal = read<uint8_t>(fileTES3->size);
            break;
        }
        case TES4:
        case FO3:
        case SSE: {
            const auto *fileTES4 = get<FileTES4 *>(fileRecord);
            seek(fileTES4->offset);
            uint32_t size = fileTES4->size;
            bool isCompressed = (size & flags::file::SIZE_COMPRESS) != 0U;
            if (isCompressed)
            {
                size &= ~flags::file::SIZE_COMPRESS;
            }
            const auto &header = get<HeaderTES4>(m_header);
            if ((header.flags & flags::archive::COMPRESS) != 0U)
            {
                isCompressed = !isCompressed;
            }

            // skip embedded file name + length prefix
            if ((m_type == FO3 || m_type == SSE) && ((header.flags & flags::archive::EMBEDNAME) != 0U))
            {
                const auto length = gsl_lite::narrow<uint32_t>(readStringLen(false).u16string().length());
                size -= length + 1;
            }

            if (isCompressed)
            {
                // reading uncompressed size
                const auto uncompressedSize = read<uint32_t>();
                size -= sizeof(uint32_t);
                if (size > 0 && uncompressedSize > 0)
                {
                    const Buffer buffer = read<uint8_t>(size);
                    if (m_multithreaded)
                    {
                        lock.unlock();
                    }
                    retVal = decompressData(buffer, uncompressedSize);
                    if (m_multithreaded)
                    {
                        lock.lock();
                    }
                }
            }
            else
            {
                if (size > 0)
                {
                    retVal = read<uint8_t>(size);
                }
            }
            break;
        }
        case FO4:
        case SF: {
            const auto *fileFO4 = get<FileFO4 *>(fileRecord);
            seek(fileFO4->offset);
            if (fileFO4->packedSize != 0)
            {
                const Buffer buffer = read<uint8_t>(fileFO4->packedSize);
                lock.unlock();
                retVal = decompressData(buffer, fileFO4->size);
                lock.lock();
            }
            else
            {
                retVal = read<uint8_t>(fileFO4->size);
            }
            break;
        }
        case FO4dds:
        case SFdds: {
            const auto *fileFO4 = get<FileFO4 *>(fileRecord);

            uint32_t texSize = sizeof(DDSHeader);
            for (const auto &chunk : fileFO4->texChunks)
            {
                texSize += chunk.size;
            }
            retVal.resize(texSize, 0);

            auto *const ddsHeader = reinterpret_cast<DDSHeader *>(retVal.data());
            ddsHeader->magic = magic::DDS;
            ddsHeader->size = sizeof(DDSHeader) - sizeof(Magic4);
            ddsHeader->width = fileFO4->width;
            ddsHeader->height = fileFO4->height;
            ddsHeader->flags = DDSD_CAPS | DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_MIPMAPCOUNT;
            ddsHeader->caps = DDSCAPS_TEXTURE;
            ddsHeader->mipMapCount = fileFO4->numMips;
            if (ddsHeader->mipMapCount > 1)
            {
                ddsHeader->caps |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
            }
            ddsHeader->depth = 1;

            auto *const ddsHeaderDX10 = reinterpret_cast<DDSHeaderDX10 *>(&retVal[sizeof(DDSHeader)]);
            ddsHeaderDX10->resourceDimension = DDS_DIMENSION_TEXTURE2D;
            ddsHeaderDX10->arraySize = 1;

            if (fileFO4->cubeMaps == 2049)
            {
                // Archive2.exe creates invalid textures like this
                //DDSHeader.dwCaps = DDSHeader.dwCaps or DDSCAPS2_CUBEMAP or DDSCAPS_COMPLEX
                //                 or DDSCAPS2_POSITIVEX or DDSCAPS2_NEGATIVEX
                //                 or DDSCAPS2_POSITIVEY or DDSCAPS2_NEGATIVEY
                //                 or DDSCAPS2_POSITIVEZ or DDSCAPS2_NEGATIVEZ;
                // This is the correct way
                ddsHeader->caps |= DDSCAPS_COMPLEX;
                ddsHeader->caps2 = DDSCAPS2_CUBEMAP | DDSCAPS2_POSITIVEX | DDSCAPS2_NEGATIVEX | DDSCAPS2_POSITIVEY
                                   | DDSCAPS2_NEGATIVEY | DDSCAPS2_POSITIVEZ | DDSCAPS2_NEGATIVEZ;
                ddsHeaderDX10->miscFlags = DDS_RESOURCE_MISC_TEXTURECUBE;
            }
            ddsHeader->ddspf.size = sizeof(DDS_PIXELFORMAT);

            switch (static_cast<DXGI_FORMAT>(fileFO4->dxgiFormat))
            {
                case DXGI_FORMAT_BC1_UNORM:
                    ddsHeader->flags |= DDSD_LINEARSIZE;
                    ddsHeader->ddspf.flags = DDPF_FOURCC;
                    ddsHeader->ddspf.fourCC = magic::DXT1;
                    ddsHeader->pitchOrLinearSize = fileFO4->width * fileFO4->height / 2;
                    break;
                case DXGI_FORMAT_BC2_UNORM:
                    ddsHeader->flags |= DDSD_LINEARSIZE;
                    ddsHeader->ddspf.flags = DDPF_FOURCC;
                    ddsHeader->ddspf.fourCC = magic::DXT3;
                    ddsHeader->pitchOrLinearSize = fileFO4->width * fileFO4->height;
                    break;

                case DXGI_FORMAT_BC3_UNORM:
                    ddsHeader->flags |= DDSD_LINEARSIZE;
                    ddsHeader->ddspf.flags = DDPF_FOURCC;
                    ddsHeader->ddspf.fourCC = magic::DXT5;
                    ddsHeader->pitchOrLinearSize = fileFO4->width * fileFO4->height;
                    break;
                case DXGI_FORMAT_BC4_SNORM:
                    ddsHeader->flags |= DDSD_LINEARSIZE;
                    ddsHeader->ddspf.flags = DDPF_FOURCC;
                    ddsHeader->ddspf.fourCC = magic::BC4S;
                    ddsHeader->pitchOrLinearSize = fileFO4->width * fileFO4->height / 2;
                    break;
                case DXGI_FORMAT_BC4_UNORM:
                    ddsHeader->flags |= DDSD_LINEARSIZE;
                    ddsHeader->ddspf.flags = DDPF_FOURCC;
                    ddsHeader->ddspf.fourCC = magic::BC4U;
                    ddsHeader->pitchOrLinearSize = fileFO4->width * fileFO4->height / 2;
                    break;
                case DXGI_FORMAT_BC5_SNORM:
                    ddsHeader->flags |= DDSD_LINEARSIZE;
                    ddsHeader->ddspf.flags = DDPF_FOURCC;
                    ddsHeader->ddspf.fourCC = magic::BC5S;
                    ddsHeader->pitchOrLinearSize = fileFO4->width * fileFO4->height;
                    break;
                case DXGI_FORMAT_BC5_UNORM:
                    ddsHeader->flags |= DDSD_LINEARSIZE;
                    ddsHeader->ddspf.flags = DDPF_FOURCC;
                    ddsHeader->ddspf.fourCC = magic::BC5U;
                    ddsHeader->pitchOrLinearSize = fileFO4->width * fileFO4->height;
                    break;
                case DXGI_FORMAT_BC1_UNORM_SRGB:
                    ddsHeader->flags |= DDSD_LINEARSIZE;
                    ddsHeader->ddspf.flags = DDPF_FOURCC;
                    ddsHeader->ddspf.fourCC = magic::DX10;
                    ddsHeaderDX10->dxgiFormat = fileFO4->dxgiFormat;
                    ddsHeader->pitchOrLinearSize = fileFO4->width * fileFO4->height / 2;
                    break;
                case DXGI_FORMAT_BC2_UNORM_SRGB:
                case DXGI_FORMAT_BC3_UNORM_SRGB:
                case DXGI_FORMAT_BC6H_UF16:
                case DXGI_FORMAT_BC6H_SF16:
                case DXGI_FORMAT_BC7_UNORM:
                case DXGI_FORMAT_BC7_UNORM_SRGB:
                    ddsHeader->flags |= DDSD_LINEARSIZE;
                    ddsHeader->ddspf.flags = DDPF_FOURCC;
                    ddsHeader->ddspf.fourCC = magic::DX10;
                    ddsHeaderDX10->dxgiFormat = fileFO4->dxgiFormat;
                    ddsHeader->pitchOrLinearSize = fileFO4->width * fileFO4->height;
                    break;
                case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
                case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
                case DXGI_FORMAT_R8G8B8A8_SINT:
                case DXGI_FORMAT_R8G8B8A8_UINT:
                case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
                    ddsHeader->flags |= DDSD_PITCH;
                    ddsHeader->ddspf.flags = DDPF_FOURCC;
                    ddsHeader->ddspf.fourCC = magic::DX10;
                    ddsHeaderDX10->dxgiFormat = fileFO4->dxgiFormat;
                    ddsHeader->pitchOrLinearSize = fileFO4->width * 4;
                    break;
                case DXGI_FORMAT_R8G8_SINT:
                case DXGI_FORMAT_R8G8_UINT:
                    ddsHeader->flags |= DDSD_PITCH;
                    ddsHeader->ddspf.flags = DDPF_FOURCC;
                    ddsHeader->ddspf.fourCC = magic::DX10;
                    ddsHeaderDX10->dxgiFormat = fileFO4->dxgiFormat;
                    ddsHeader->pitchOrLinearSize = fileFO4->width * 2;
                    break;
                case DXGI_FORMAT_R8_SINT:
                case DXGI_FORMAT_R8_SNORM:
                case DXGI_FORMAT_R8_UINT:
                    ddsHeader->flags |= DDSD_PITCH;
                    ddsHeader->ddspf.flags = DDPF_FOURCC;
                    ddsHeader->ddspf.fourCC = magic::DX10;
                    ddsHeaderDX10->dxgiFormat = fileFO4->dxgiFormat;
                    ddsHeader->pitchOrLinearSize = fileFO4->width;
                    break;
                case DXGI_FORMAT_R8G8B8A8_UNORM:
                    ddsHeader->flags |= DDSD_PITCH;
                    ddsHeader->ddspf.flags = DDPF_RGB | DDPF_ALPHAPIXELS;
                    ddsHeader->ddspf.RGBBitCount = 32;
                    ddsHeader->ddspf.RBitMask = 0x000000FF;
                    ddsHeader->ddspf.GBitMask = 0x0000FF00;
                    ddsHeader->ddspf.BBitMask = 0x00FF0000;
                    ddsHeader->ddspf.ABitMask = 0xFF000000;
                    ddsHeader->pitchOrLinearSize = fileFO4->width * 4;
                    break;
                case DXGI_FORMAT_B8G8R8A8_UNORM:
                    ddsHeader->flags |= DDSD_PITCH;
                    ddsHeader->ddspf.flags = DDPF_RGB | DDPF_ALPHAPIXELS;
                    ddsHeader->ddspf.RGBBitCount = 32;
                    ddsHeader->ddspf.RBitMask = 0x00FF0000;
                    ddsHeader->ddspf.GBitMask = 0x0000FF00;
                    ddsHeader->ddspf.BBitMask = 0x000000FF;
                    ddsHeader->ddspf.ABitMask = 0xFF000000;
                    ddsHeader->pitchOrLinearSize = fileFO4->width * 4;
                    break;
                case DXGI_FORMAT_B8G8R8X8_UNORM:
                    ddsHeader->flags |= DDSD_PITCH;
                    ddsHeader->ddspf.flags = DDPF_RGB;
                    ddsHeader->ddspf.RGBBitCount = 32;
                    ddsHeader->ddspf.RBitMask = 0x00FF0000;
                    ddsHeader->ddspf.GBitMask = 0x0000FF00;
                    ddsHeader->ddspf.BBitMask = 0x000000FF;
                    ddsHeader->pitchOrLinearSize = fileFO4->width * 4;
                    break;
                case DXGI_FORMAT_B5G6R5_UNORM:
                    ddsHeader->flags |= DDSD_PITCH;
                    ddsHeader->ddspf.flags = DDPF_RGB;
                    ddsHeader->ddspf.RGBBitCount = 16;
                    ddsHeader->ddspf.RBitMask = 0x0000F800;
                    ddsHeader->ddspf.GBitMask = 0x000007E0;
                    ddsHeader->ddspf.BBitMask = 0x0000001F;
                    ddsHeader->pitchOrLinearSize = fileFO4->width * 2;
                    break;
                case DXGI_FORMAT_B5G5R5A1_UNORM:
                    ddsHeader->flags |= DDSD_PITCH;
                    ddsHeader->ddspf.flags = DDPF_RGB | DDPF_ALPHAPIXELS;
                    ddsHeader->ddspf.RGBBitCount = 16;
                    ddsHeader->ddspf.RBitMask = 0x00007C00;
                    ddsHeader->ddspf.GBitMask = 0x000003E0;
                    ddsHeader->ddspf.BBitMask = 0x0000001F;
                    ddsHeader->ddspf.ABitMask = 0x00008000;
                    ddsHeader->pitchOrLinearSize = fileFO4->width * 2;
                    break;
                case DXGI_FORMAT_R8G8_UNORM:
                    ddsHeader->flags |= DDSD_PITCH;
                    ddsHeader->ddspf.flags = DDPF_LUMINANCE | DDPF_ALPHAPIXELS;
                    ddsHeader->ddspf.RGBBitCount = 16;
                    ddsHeader->ddspf.RBitMask = 0x000000FF;
                    ddsHeader->ddspf.ABitMask = 0x0000FF00;
                    ddsHeader->pitchOrLinearSize = fileFO4->width * 2;
                    break;
                case DXGI_FORMAT_A8_UNORM:
                    ddsHeader->flags |= DDSD_PITCH;
                    ddsHeader->ddspf.flags = DDPF_ALPHA;
                    ddsHeader->ddspf.RGBBitCount = 8;
                    ddsHeader->ddspf.ABitMask = 0x000000FF;
                    ddsHeader->pitchOrLinearSize = fileFO4->width;
                    break;
                case DXGI_FORMAT_R8_UNORM:
                    ddsHeader->flags |= DDSD_PITCH;
                    ddsHeader->ddspf.flags = DDPF_LUMINANCE;
                    ddsHeader->ddspf.RGBBitCount = 8;
                    ddsHeader->ddspf.RBitMask = 0x000000FF;
                    ddsHeader->pitchOrLinearSize = fileFO4->width;
                    break;
                default:
                    //don't do anything
                    break;
            }

            texSize = sizeof(DDSHeader);
            if (ddsHeader->ddspf.fourCC == magic::DX10)
            {
                retVal.resize(retVal.size() + sizeof(DDSHeaderDX10), 0);
                texSize += sizeof(DDSHeaderDX10);
            }

            // append chunks
            for (const auto &chunk : fileFO4->texChunks)
            {
                seek(chunk.offset);
                // compressed chunk
                if (chunk.packedSize != 0)
                {
                    Buffer buffer = read<uint8_t>(chunk.packedSize);
                    lock.unlock();
                    decompressData(buffer, retVal.data() + texSize, chunk.size);
                    lock.lock();
                }
                // uncompressed chunk
                else
                {
                    read<uint8_t>(retVal.data() + texSize, chunk.size);
                }
                texSize += chunk.size;
            }

            break;
        }
        [[unlikely]] default:
            throw runtime_error("Extraction is not supported for this archive");
    }

    return retVal;
}

void Bsa::packData(const FileRecord_t &fileRecord,
                   const filesystem::path &filePath,
                   const PackedDataHash &dataHash,
                   const uint8_t *data,
                   uint32_t size,
                   bool compress,
                   const bool doCompress) noexcept(false)
{
    // check if data already exists
    if (findPackedData(size, dataHash, fileRecord))
    {
        return;
    }

    // uncompressed size in case the provided data gets compressed
    const uint32_t uncompressedSize = size;

    Buffer compressedBuffer;

    try
    {
        if (compress)
        {
            // compressing in parallel when multithreaded
            unlock();
            compressedBuffer = compressData(data, size);
            lock();

            // leave as compressed if compression reduced the size by at least 32 bytes
            // or data is forced to be compressed
            if (doCompress || compressedBuffer.size() + sizes::compressionMinimumDifference < size)
            {
                data = compressedBuffer.data();
                size = compressedBuffer.size();
            }
            else
            {
                compress = false;
            }
        }
        // try to find existing data again if multithreaded
        // maybe some other thread has written the same data while we've been busy compressing
        if (m_multithreaded)
        {
            if (findPackedData(size, dataHash, fileRecord))
            {
                return;
            }
        }

        const int64_t position = _ftelli64(m_file.get());

        // embedded name for Fallout 3/NV/Skyrim/Skyrim SE
        if ((m_type == FO3 || m_type == SSE) && (get<HeaderTES4>(m_header).flags & flags::archive::EMBEDNAME) != 0)
        {
            writeStringLen8(filePath, false);
        }

        // if compressed then write uncompressed size first for Oblivion/Fallout 3/NV/Skyrim/Skyrim SE
        if ((m_type == TES4 || m_type == FO3 || m_type == SSE) && compress)
        {
            write(uncompressedSize);
        }

        write(data, size);

        // updating file record
        switch (m_type)
        {
            case TES3: {
                auto *const file = get<FileTES3 *>(fileRecord);

                try
                {
                    file->offset = gsl_lite::narrow<uint32_t>(position);
                }
                catch (const gsl_lite::narrowing_error &)
                {
                    // xEdit allows creating archives > 4GiB which I assume is an error
                    m_abort.store(true);
                    throw runtime_error("Error packing data: Archive exceeds 4GiB");
                }
                file->size = uncompressedSize;
                break;
            }
            case TES4:
            case FO3:
            case SSE: {
                auto *const file = get<FileTES4 *>(fileRecord);

                try
                {
                    file->offset = gsl_lite::narrow<uint32_t>(position);
                    file->size = gsl_lite::narrow<uint32_t>(_ftelli64(m_file.get()) - position);
                }
                catch (const gsl_lite::narrowing_error &)
                {
                    // xEdit allows creating archives > 4GiB which I assume is an error
                    throw runtime_error("Error packing data: Archive exceeds 4GiB");
                }
                // compress flag in Size inverts compression status from the header
                // set it if archive's compression doesn't match file's compression
                if (m_compressed ^ compress)
                {
                    file->size |= flags::file::SIZE_COMPRESS;
                }
                break;
            }
            case FO4:
            case SF: {
                auto *const file = get<FileFO4 *>(fileRecord);

                file->offset = position;
                file->size = uncompressedSize;
                if (compress)
                {
                    file->packedSize = gsl_lite::narrow<uint32_t>(size);
                }
                break;
            }
            case FO4dds:
            case SFdds: {
                auto *const chunk = get<TexChunkRec *>(fileRecord);

                chunk->offset = position;
                chunk->size = uncompressedSize;
                if (compress)
                {
                    chunk->packedSize = gsl_lite::narrow<uint32_t>(size);
                }
                break;
            }
            [[unlikely]] default:
                throw runtime_error("Invalid archive type for packing");
        }

        addPackedData(uncompressedSize, dataHash, fileRecord);
    }
    catch (const exception &ex)
    {
        throw runtime_error("Exception while packing: "s + ex.what());
    }
    catch (...)
    {
        throw runtime_error("Exception while packing");
    }
}

Buffer Bsa::compressData(const Buffer &data) const noexcept(false)
{
    return compressData(data.data(), data.size());
}

Buffer Bsa::compressData(const uint8_t *data, size_t length) const noexcept(false)
{
    Buffer dst;

    switch (m_compressionType)
    {
        case zlib: {
            // get worst case output size
            uLong size = compressBound(gsl_lite::narrow<uLong>(length));
            dst.resize(size);

            int result = compress2(dst.data(), &size, data, gsl_lite::narrow<uLong>(length), m_compressionLevel);

            if (result != Z_OK)
            {
                // compress returned either Z_MEM_ERROR or Z_BUF_ERROR
                throw runtime_error("Compression failed: "s
                                    + (result == Z_MEM_ERROR ? "Not enough memory"s : "Output buffer is too small"s));
            }
            // resize buffer to compressed size
            dst.resize(size);

            break;
        }
        case lz4Frame: {
            static LZ4F_preferences_t preferences = LZ4F_INIT_PREFERENCES;
            static bool preferencesInitialized = false;
            if (!preferencesInitialized) // only create preferences once
            {
                if (m_compressionLevel == 0)
                {
                    // compression level has not been set, use default value
                    preferences.compressionLevel = lz4CompressionLevel;
                }
                preferences.frameInfo.blockSizeID = LZ4F_max4MB;
                preferences.frameInfo.blockMode = LZ4F_blockIndependent;
                preferences.frameInfo.contentChecksumFlag = LZ4F_contentChecksumEnabled;
                preferencesInitialized = true;
            }

            // get worst case output size
            const size_t outBufCapacity = LZ4F_compressBound(length, &preferences);
            // sanity check
            if (outBufCapacity < LZ4F_HEADER_SIZE_MAX)
            {
                throw runtime_error("Compression error: output Buffer is too small for LZ4F header");
            }

            dst.resize(outBufCapacity);

            // compress
            const size_t result = LZ4F_compressFrame(dst.data(), outBufCapacity, data, length, &preferences);
            //check for errors
            if (LZ4F_isError(result) != 0U)
            {
                throw runtime_error("Compression error: "s + LZ4F_getErrorName(result));
            }
            // resize buffer to compressed size
            dst.resize(result);

            break;
        }
        case lz4Block: {
            if (length > LZ4_MAX_INPUT_SIZE)
            {
                throw runtime_error("Compression error: data size > LZ4MAX_INPUT_SIZE");
            }

            const int srcSize = gsl_lite::narrow<int>(length);

            // get worst case output size
            const int bound = LZ4_compressBound(srcSize);
            dst.resize(bound);

            // compress
            const int result = LZ4_compress_default(reinterpret_cast<const char *>(data),
                                                    reinterpret_cast<char *>(dst.data()),
                                                    srcSize,
                                                    bound);

            // check for errors
            if (result == 0)
            {
                throw runtime_error("Compression error");
            }
            // resize buffer to compressed size
            dst.resize(result);

            break;
        }
    }

    dst.shrink_to_fit();
    return dst;
}

std::vector<std::filesystem::path> Bsa::getFileList(
    const std::optional<std::filesystem::path> &directoryName) const noexcept
{
    vector<fs::path> result;
    fs::path directory;
    if (directoryName.has_value())
    {
        directory = canonical(directoryName.value());
    }

    const string folderLower = ToLower(directory.string());

    switch (m_type)
    {
        case TES3:
            result.reserve(m_files.size());
            for (const auto &_file : m_files)
            {
                const auto &file = get<FileTES3>(_file);
                if (directory.empty() || ToLower(file.name).starts_with(folderLower))
                {
                    result.emplace_back(file.name);
                }
            }
            break;
        case TES4:
        case FO3:
        case SSE:
            result.reserve(m_files.size());
            for (const auto &_file : m_files)
            {
                const auto &folder = get<FolderTES4>(_file);
                if (directory.empty() || ToLower(folder.name).starts_with(folderLower))
                {
                    for (const auto &fileTES4 : folder.files)
                    {
                        result.emplace_back(folder.name / fileTES4.name);
                    }
                }
            }

            break;
        case FO4:
        case FO4dds:
        case SF:
        case SFdds:
            result.reserve(m_files.size());
            for (const auto &_file : m_files)
            {
                const auto &file = get<FileFO4>(_file);
                if (directory.empty() || ToLower(file.name).starts_with(folderLower))
                {
                    result.emplace_back(file.name);
                }
            }
            break;
        case none: break;
    }

    result.shrink_to_fit();

    return result;
}

DDSInfo Bsa::getDDSInfo(const std::filesystem::path &filePath) const noexcept(false)
{
    const fs::path fullFilePath = m_ddsBasePath / filePath;

    const filePtr ddsFile(fopen(fullFilePath.string().c_str(), "rb"));

    if (ddsFile.get() == nullptr)
    {
        const int error = errno;
        throw runtime_error(format("Error reading {}: {}", fullFilePath.string(), strerror(error)));
    }

    DDSHeader header;
    if (fread(&header, sizeof(DDSHeader), 1, ddsFile.get()) != 1 || header.magic != magic::DDS)
    {
        const int error = errno;
        throw runtime_error(format("Error reading dds file {}: {}", fullFilePath.string(), strerror(error)));
    }

    DDSInfo info;
    info.height = gsl_lite::narrow<int32_t>(header.height);
    info.width = gsl_lite::narrow<int32_t>(header.width);
    info.mipMaps = gsl_lite::narrow<int32_t>(header.mipMapCount);

    return info;
}

void Bsa::lock() noexcept
{
    if (m_multithreaded)
    {
        m_writeMtx.lock();
    }
}

void Bsa::unlock() noexcept
{
    if (m_multithreaded)
    {
        m_writeMtx.unlock();
    }
}

void Bsa::seek(int64_t pos, SeekDirection whence) const noexcept(false)
{
    if (_fseeki64(m_file.get(), pos, whence) != 0)
    {
        const int error = errno;
        throw runtime_error("Seek error: "s + strerror(error));
    }
}

HeaderFO4 &Bsa::getHeaderFO4() noexcept(false)
{
    if (holds_alternative<HeaderFO4>(m_header))
    {
        return get<HeaderFO4>(m_header);
    }
    if (holds_alternative<HeaderSF>(m_header))
    {
        return get<HeaderSF>(m_header).fo4Header;
    }
    if (holds_alternative<HeaderSFdds>(m_header))
    {
        return get<HeaderSFdds>(m_header).fo4Header;
    }
    throw std::runtime_error("Archive does not contain a FO4 Header");
}

void Bsa::extract(const filesystem::path &archivePath,
                  const filesystem::path &outputDirectory,
                  const bool multithreaded) noexcept(false)
{
    Bsa bsa(archivePath, true);
    auto files = bsa.getFileList();

    if (multithreaded)
    {
        vector<string> exceptions;
        mutex mtx;

        for_each(std::execution::par, files.begin(), files.end(), [&](const fs::path &file) {
            try
            {
                bsa.extractFile(file, outputDirectory / file);
            }
            catch (const runtime_error &ex)
            {
                // std::terminate is called if exceptions are not caught inside for_each
                lock_guard guard(mtx);
                exceptions.emplace_back(ex.what());
            }
        });
        if (!exceptions.empty())
        {
            throw runtime_error("Error: "s + exceptions[0]);
        }
    }
    else
    {
        for (const auto &file : files)
        {
            bsa.extractFile(file, outputDirectory / file);
        }
    }
}

void Bsa::create(const std::filesystem::path &archivePath,
                 ArchiveType type,
                 const std::filesystem::path &inputDirectory,
                 BsaCreationSettings settings) noexcept(false)
{
    if (exists(archivePath))
    {
        throw runtime_error("Archive file " + archivePath.string() + " already exists");
    }

    vector<fs::path> files;

    for (const auto &entry : fs::recursive_directory_iterator(inputDirectory))
    {
        if (entry.is_regular_file())
        {
            string ext = entry.path().extension().string();
            ToLowerInline(ext);

            // only pack dds files for dds archives
            if ((type == FO4dds || type == SFdds) && ext != ".dds")
            {
                continue;
            }

            bool excluded = ranges::find(settings.extensionBlacklist, ext) != settings.extensionBlacklist.end();

            if ((ext.empty() && settings.ignoreExtensionlessFiles) || excluded)
            {
                continue;
            }
            fs::path relative = entry.path().generic_string().substr(inputDirectory.generic_string().size() + 1);
            files.emplace_back(relative);
        }
    }

    if (files.empty())
    {
        throw runtime_error("No valid files found for packing");
    }

    Bsa bsa(archivePath, type, files, inputDirectory, settings.compressed, settings.shareData, settings.multithreaded);
    bsa.setCompressionLevel(settings.compressionLevel);

    try
    {
        if (settings.multithreaded)
        {
            vector<string> exceptions;
            mutex mtx;
            for_each(std::execution::par, files.begin(), files.end(), [&](const auto &file) {
                try
                {
                    bsa.addFile(inputDirectory, inputDirectory / file);
                }
                catch (const runtime_error &ex)
                {
                    // std::terminate is called if exceptions are not caught inside for_each
                    lock_guard guard(mtx);
                    exceptions.emplace_back(ex.what());
                }
            });
            if (!exceptions.empty())
            {
                throw runtime_error("Error: "s + exceptions[0]);
            }
        }
        else
        {
            for (const auto &file : files)
            {
                bsa.addFile(inputDirectory, inputDirectory / file);
            }
        }
    }
    catch (...)
    {
        throw;
    }

    bsa.save();
}

} // namespace libbsarchpp