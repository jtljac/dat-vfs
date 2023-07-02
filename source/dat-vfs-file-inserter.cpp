//
// Created by jacob on 02/07/23.
//
#include "../include/dat-vfs-file-inserter.h"

#include <functional>

void DVFS::IDVFSFileInserter::handleInsertFailure(const std::string& path, DVFS::IDVFSFile* idvfsFile) const {
    delete idvfsFile;
}

std::vector<DVFS::DVFSLooseFileInserter::pair> DVFS::DVFSLooseFileInserter::getAllFiles() const {
    std::vector<pair> files;

    for (const auto& it: std::filesystem::recursive_directory_iterator(directory)) {
        if (it.is_directory()) continue;
        files.emplace_back(std::filesystem::relative(it.path(), directory).string(), new LooseDVFSFile(it.path()));
    }

    return files;
}
