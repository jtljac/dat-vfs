#include "../include/DatVfsFileInserter.h"

#include <functional>

void Dvfs::IDvfsFileInserter::handleInsertFailure(const std::string& path, Dvfs::IDvfsFile* idvfsFile) const {
    delete idvfsFile;
}

std::vector<Dvfs::DvfsLooseFileInserter::pair> Dvfs::DvfsLooseFileInserter::getAllFiles() const {
    std::vector<pair> files;

    for (const auto& it: std::filesystem::recursive_directory_iterator(directory)) {
        if (it.is_directory()) continue;
        files.emplace_back(std::filesystem::relative(it.path(), directory).string(), new LooseDvfsFile(it.path()));
    }

    return files;
}
