#pragma once

#include <string>

namespace DVFS {
/**
 * A wrapper for strings that represents paths in a DatVFS
 * <br>
 * This provides some extra functions to make it easier to operate on these classes internally
 */
    class DatPath {
        std::string path;

        static std::string sanitisePath(std::string path);

    public:
        /**
         * Create an empty path
         */
        DatPath() = default;

        /**
         * Create a path
         * @param path A string representing the path
         */
        DatPath(std::string path) : path(sanitisePath(std::move(path))) {} // NOLINT(google-explicit-constructor)
        DatPath(const char* path) : DatPath(std::string(path)) {} // NOLINT(google-explicit-constructor)

        explicit operator std::string() const;

        bool operator==(const DatPath& rh) const;

        /**
         * Append the sub-path onto the existing path
         * @param subPath The path to append
         * @return A new DatPath made out of the existing lh one and the given rh string
         */
        DatPath operator /(const std::string& subPath) const;

        /**
         * Append the sub-path onto the existing path
         * @param subPath The path to append
         * @return A new DatPath made out of the existing lh one and the given rh string
         */
        DatPath operator /(const char* subPath) const;

        /**
         * Append the sub-path onto the existing path
         * @param subPath The path to append
         * @return A new DatPath made out of the existing lh one and the given rh one
         */
        DatPath operator /(const DatPath& subPath) const;

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

template <>
struct std::hash<DVFS::DatPath>
{
    std::size_t operator()(const DVFS::DatPath& k) const
    {
        return hash<std::string>()((std::string) k);
    }
};