//
// Created by jacob on 29/06/23.
//

#pragma once

#include <string>

namespace DatVFS {
/**
 * A wrapper for strings that represents paths in a DatVFS
 * <br>
 * This provides some extra functions to make it easier to operate on these classes internally
 */
    class DatPath {
        std::string path;

    public:
        /**
         * Create an empty path
         */
        DatPath() = default;

        /**
         * Create a path
         * @param path A string representing the path
         */
        DatPath(const std::string& path); // NOLINT(google-explicit-constructor)

        DatPath(DatPath& path) = default;

        explicit operator std::string() const;

        bool operator==(const DatPath& rh) const;

        /**
         * Go up a level in the path, the given number of directories from the beginning of the path
         * @param levels The number of directories to remove
         * @return a new path with the given number of directories removed
         */
        [[nodiscard]] DatPath increment(size_t levels = 1) const;

        /**
         * Get a path representing the root of this path
         * @return a new path representing the root of this path
         */
        [[nodiscard]] DatPath getRoot() const;

        /**
         * Get the number of sections in the path
         * @return the depth of the path
         */
        [[nodiscard]] size_t depth() const;

        /**
         * Check if the path is empty
         * @return True if the path is empty
         */
        [[nodiscard]] bool empty() const;
    };
}