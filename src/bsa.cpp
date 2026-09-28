#include "bsa.h"
#include "bsaimpl.h"

#include "enums.h"  // for ArchiveType, CompressionType
#include "types.h"  // for FileFO4, FolderTES4, FileTES3

using namespace std;
namespace fs = std::filesystem;

namespace libbsarchpp {

void Bsa::extract(const std::filesystem::path& archivePath, const std::filesystem::path& outputDirectory,
                  bool multithreaded) noexcept(false) {
  BsaImpl::extract(archivePath, outputDirectory, multithreaded);
}

void Bsa::create(const std::filesystem::path& archivePath, ArchiveType type,
                 const std::filesystem::path& inputDirectory, const BsaCreationSettings& settings) noexcept(false) {
  BsaImpl::create(archivePath, type, inputDirectory, settings);
}

ArchiveType Bsa::getArchiveType(const std::filesystem::path& archivePath) noexcept(false) {
  return BsaImpl::getArchiveType(archivePath);
}

Bsa::Bsa(const std::filesystem::path& archivePath, bool multithreaded) noexcept(false) {
  m_impl = new BsaImpl(archivePath, multithreaded);
}

Bsa::Bsa(const std::filesystem::path& archivePath, ArchiveType type, std::vector<std::filesystem::path>& fileList,
         std::filesystem::path ddsBasePath, bool compressed, bool shareData, bool multithreaded) noexcept(false) {
  m_impl = new BsaImpl(archivePath, type, fileList, std::move(ddsBasePath), compressed, shareData, multithreaded);
}

Bsa::Bsa(const std::filesystem::path& archivePath, ArchiveType type, std::vector<std::filesystem::path>& fileList,
         std::filesystem::path ddsBasePath) noexcept(false) {
  m_impl = new BsaImpl(archivePath, type, fileList, std::move(ddsBasePath));
}

Bsa::~Bsa() {
  delete m_impl;
}

void Bsa::save() noexcept(false) {
  m_impl->save();
}

void Bsa::addFile(const std::filesystem::path& rootDirectory, const std::filesystem::path& filePath) noexcept(false) {
  m_impl->addFile(rootDirectory, filePath);
}

void Bsa::addFile(const std::filesystem::path& filePath, const Buffer& data) noexcept(false) {
  m_impl->addFile(filePath, data);
}

FileRecord_t Bsa::findFileRecord(const std::filesystem::path& fileName) noexcept {
  return m_impl->findFileRecord(fileName);
}

Buffer Bsa::extractFileData(const FileRecord_t& fileRecord) noexcept(false) {
  return m_impl->extractFileData(fileRecord);
}

Buffer Bsa::extractFileData(const std::filesystem::path& fileName) noexcept(false) {
  return m_impl->extractFileData(fileName);
}

void Bsa::extractFile(const std::filesystem::path& filePath, const std::filesystem::path& saveAs) noexcept(false) {
  m_impl->extractFile(filePath, saveAs);
}

void Bsa::iterateFiles(const FileIterationFunction& function, void* data) noexcept {
  m_impl->iterateFiles(function, data);
}

bool Bsa::fileExists(const std::filesystem::path& filePath) noexcept {
  return m_impl->fileExists(filePath);
}

std::vector<std::filesystem::path> Bsa::getFileList(const std::filesystem::path& directoryName) const noexcept {
  return m_impl->getFileList(directoryName);
}

std::filesystem::path Bsa::getFileName() const noexcept {
  return m_impl->getFileName();
}

ArchiveType Bsa::getArchiveType() const noexcept {
  return m_impl->getArchiveType();
}

HeaderVersion Bsa::getVersion() const noexcept {
  return m_impl->getVersion();
}

std::string Bsa::getArchiveFormatName() const noexcept {
  return m_impl->getArchiveFormatName();
}

uint32_t Bsa::getFileCount() const noexcept {
  return m_impl->getFileCount();
}

int64_t Bsa::getCreatedArchiveSize() const noexcept {
  return m_impl->getCreatedArchiveSize();
}

uint32_t Bsa::getArchiveFlags() const noexcept {
  return m_impl->getArchiveFlags();
}

void Bsa::setArchiveFlags(uint32_t flags) noexcept(false) {
  return m_impl->setArchiveFlags(flags);
}

bool Bsa::getCompressed() const noexcept {
  return m_impl->getCompressed();
}

void Bsa::setCompressed(bool value) noexcept {
  m_impl->setCompressed(value);
}

bool Bsa::getShareData() const noexcept {
  return m_impl->getShareData();
}

void Bsa::setShareData(bool value) noexcept {
  m_impl->setShareData(value);
}

void Bsa::setDDSBasePath(const std::filesystem::path& path) noexcept {
  m_impl->setDDSBasePath(path);
}

bool Bsa::isMultithreaded() const noexcept {
  return m_impl->isMultithreaded();
}

void Bsa::setMultithreading(bool value) noexcept {
  m_impl->setMultithreading(value);
}

int Bsa::getCompressionLevel() const noexcept {
  return m_impl->getCompressionLevel();
}

void Bsa::setCompressionLevel(int value) noexcept {
  m_impl->setCompressionLevel(value);
}

}  // namespace libbsarchpp
