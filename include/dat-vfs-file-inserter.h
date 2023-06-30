//
// Created by jacob on 29/06/23.
//

#pragma once

#include <string>
#include <functional>
#include <utility>
#include "dat-vfs-file.h"

namespace DVFS {
    /**
     * An interface for inserting files into a VFS
     */
    struct IDVFSFileInserter {
        using pair = std::pair<std::string, IDVFSFile*>;

        /**
         * Gets a vector containing all the DVFS files, paired with their path, to insert into a VFS
         * @return A vector of DVFS files to be inserted into a VFS
         */
        [[nodiscard]] virtual std::vector<pair> getAllFiles() const = 0;

        /**
         * Handle files that failed to be inserted into the VFS
         * @param path The path of the file
         * @param idvfsFile The file that failed to insert
         */
        virtual void handleInsertFailure(const std::string& path, IDVFSFile* idvfsFile) const {
            delete idvfsFile;
        }
    };

    struct DVFSLooseFileInserter : public IDVFSFileInserter {
        std::filesystem::path directory;
        DVFSLooseFileInserter(std::filesystem::path  directory) : directory(std::move(directory)) {}

        [[nodiscard]] std::vector<pair> getAllFiles() const override {
            std::vector<pair> files;

            for (const auto& it: std::filesystem::recursive_directory_iterator(directory)) {
                if (it.is_directory()) continue;
                files.emplace_back(std::filesystem::relative(it.path(), directory).string(), new LooseDVFSFile(it.path()));
            }

            return files;
        }
    };
}