//
// Created by jacob on 02/07/23.
//

#include "../include/dat-vfs-file.h"

#include <fstream>

uint8_t DVFS::IDVFSFile::incrementReferences() {
    return ++references;
}

uint8_t DVFS::IDVFSFile::decrementReferences() {
    return --references;
}

uint8_t DVFS::IDVFSFile::getReferenceCount() const {
    return references;
}

uint64_t DVFS::LooseDVFSFile::fileSize() const {
    return isValidFile() ? file_size(filePath) : 0;
}

bool DVFS::LooseDVFSFile::isValidFile() const {
    return !filePath.empty() && exists(filePath) && !is_directory(filePath);
}

bool DVFS::LooseDVFSFile::getContent(char* buffer) const {
    if (!isValidFile()) return false;

    std::ifstream fileStream(filePath, std::ios::in | std::ios::binary | std::ios::ate);

    std::streamsize fileSize = fileStream.tellg();
    fileStream.seekg(0);

    return fileStream.read(buffer, fileSize).good();
}
