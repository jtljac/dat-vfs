//
// Created by jacob on 28/06/23.
//

#pragma once

#include <cstdint>
#include <filesystem>

namespace DVFS {
    /**
     * An interface for files stored inside the VFS
     */
    class IDVFSFile {
        /**
         * Reference counting for hard links
         * <br>
         * This value counts the number of times the DVFSFile is stored in a VFS. When it is 0, it means no VFS stores
         * it and it can be removed (usually by the VFS that unmounted it).
         */
        uint8_t references = 0;
    public:
        virtual ~IDVFSFile() = default;

        /**
         * Increment the number of references to this DVFSFile there are
         * <br>
         * Warning, this function should only be used by the DatVFS, you probably shouldn't touch it
         * @return The new number of references that exist for this DVFSFile
         */
        uint8_t incrementReferences();

        /**
         * Decrement the number of references to this DVFSFile there are
         * <br>
         * Warning, this function should only be used by the DatVFS, you probably shouldn't touch it
         * @return The new number of references that exist for this DVFSFile
         */
        uint8_t decrementReferences();

        /**
         * Get the number of references to this DVFSFile there are
         * @return The current number of references that exist for this DVFSFile
         */
        [[nodiscard]] uint8_t getReferenceCount() const;

        /**
         * Get the size of the file
         * <br>
         * This is the full uncompressed size as would be required to hold the raw file in memory
         * @return The size of the file
         */
        [[nodiscard]] virtual uint64_t fileSize() const = 0;

        /**
         * Check the file is valid and able to be fetched
         * @return True if the file is valid
         */
        [[nodiscard]] virtual bool isValidFile() const = 0;

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
        /**
         * Create a LooseDVFSFile pointing to the file at the given path
         * @param filePath The path to the file on disk
         */
        LooseDVFSFile(std::filesystem::path  filePath) : filePath(std::move(filePath)) {} // NOLINT(google-explicit-constructor)

        /** @inherit */
        [[nodiscard]] uint64_t fileSize() const override;

        /** @inherit */
        [[nodiscard]] bool isValidFile() const override;

        /** @inherit */
        bool getContent(char* buffer) const override;
    };
}