//
// Created by jacob on 28/06/23.
//

#pragma once

#include <cstdint>
#include <fstream>

namespace DatVFS {
    /**
     * An interface for files stored inside the VFS
     */
    class IDVFSFile {
        uint8_t references = 0;

    public:

        virtual ~IDVFSFile() = default;
        /**
         * Get the size of the file
         * <br>
         * This is the full uncompressed size as would be required to hold the raw file in memory
         * @return The size of the file
         */
        virtual uint64_t fileSize() const = 0;

        /**
         * Check the file is valid and able to be fetched
         * @return True if the file is valid
         */
        virtual bool isValidFile() const = 0;

        /**
         * Get the content of the file and store it in the given buffer
         * <br>
         * Warning, this assumes the buffer is large enough for the file
         * <br>
         * The size of the file can be acquired using fileSize()
         * @param buffer The buffer to store the file in
         * @return True if successful
         */
        virtual bool getContent(char* buffer) const = 0;
    };

    /**
     * A DVFSFile implementation representing a file in the user's filesystem
     */
    class LooseDVFSFile : public IDVFSFile {
        /** The path to the file on disk */
        std::filesystem::path filePath;

    public:
        uint64_t fileSize() const override {
            return isValidFile() ? file_size(filePath) : 0;
        }

        bool isValidFile() const override {
            return !filePath.empty() && exists(filePath) && !is_directory(filePath);
        }

        bool getContent(char* buffer) const override {
            if (!isValidFile()) return false;

            std::ifstream fileStream(filePath, std::ios::in | std::ios::binary | std::ios::ate);

            std::streamsize fileSize = fileStream.tellg();
            fileStream.seekg(0);

            return fileStream.read(buffer, fileSize).good();
        }
    };
}