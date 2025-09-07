#pragma once

#include <span>
#include <string>
#include <vector>

namespace Dvfs {
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

        [[nodiscard]] std::vector<std::string_view> split() const;
    };
}

template <>
struct std::hash<Dvfs::DatPath>
{
    std::size_t operator()(const Dvfs::DatPath& k) const noexcept {
        return hash<std::string>()(static_cast<std::string>(k));
    }
};