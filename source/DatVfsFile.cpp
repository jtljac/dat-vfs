#include "../include/DatVfsFile.h"

#include <fstream>

uint8_t Dvfs::IDvfsFile::incrementReferences() {
    return ++references;
}

uint8_t Dvfs::IDvfsFile::decrementReferences() {
    return --references;
}

uint8_t Dvfs::IDvfsFile::getReferenceCount() const {
    return references;
}

uint64_t Dvfs::LooseDvfsFile::fileSize() const {
    return isValidFile() ? file_size(filePath) : 0;
}

bool Dvfs::LooseDvfsFile::isValidFile() const {
    return !filePath.empty() && exists(filePath) && !is_directory(filePath);
}

bool Dvfs::LooseDvfsFile::getContent(char* buffer) const {
    if (!isValidFile()) return false;

    std::ifstream fileStream(filePath, std::ios::in | std::ios::binary | std::ios::ate);

    std::streamsize fileSize = fileStream.tellg();
    fileStream.seekg(0);

    return fileStream.read(buffer, fileSize).good();
}
