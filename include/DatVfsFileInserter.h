#pragma once

#include <string>
#include <vector>

#include "DatVfsFile.h"

namespace Dvfs {
    /**
     * An interface for inserting files into a VFS
     */
    struct IDvfsFileInserter {
        using pair = std::pair<std::string, IDvfsFile*>;

        /**
         * Gets a vector containing all the Dvfs files, paired with their path, to insert into a VFS
         * @return A vector of Dvfs files to be inserted into a VFS
         */
        [[nodiscard]] virtual std::vector<pair> getAllFiles() const = 0;

        /**
         * Handle files that failed to be inserted into the VFS
         * @param path The path of the file
         * @param idvfsFile The file that failed to insert
         */
        virtual void handleInsertFailure(const std::string& path, IDvfsFile* idvfsFile) const;
    };

    struct DvfsLooseFileInserter : public IDvfsFileInserter {
        std::filesystem::path directory;
        DvfsLooseFileInserter(std::filesystem::path  directory) : directory(std::move(directory)) {}

        /** @inherit */
        [[nodiscard]] std::vector<pair> getAllFiles() const override;
    };
}