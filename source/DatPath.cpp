#include "../include/DatPath.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <ranges>
#include <vector>

std::string Dvfs::DatPath::sanitisePath(std::string path) {
    if (path.empty()) {
        return std::move(path);
    }

    // Ensure backslashes aren't used
    assert(path.find('\\') == std::string::npos);

    std::string::size_type startIndex = path.find_first_not_of('/');
    std::string::size_type endIndex = path.find_last_not_of('/');

    return path.substr(startIndex,
                             endIndex != std::string::npos
                             ? endIndex + 1 - startIndex
                             : std::string::npos);
}

Dvfs::DatPath::operator std::string() const {
    return path;
}

bool Dvfs::DatPath::operator==(const DatPath& rh) const {
    return path == rh.path;
}

Dvfs::DatPath Dvfs::DatPath::operator/(const std::string& subPath) const {
    // Avoid sanitising twice by creating an empty DatPath and setting its path manually, we have to sanitise once this
    // way as we don't sanitise the middle of the path, only the ends
    DatPath newPath;
    // If this path/sub-path is empty then we have to make sure we don't append a "/" at the beginning/end
    newPath.path = empty()
            ? sanitisePath(subPath)
            : subPath.empty()
                    ? path
                    : path + "/" + sanitisePath(subPath);
    return newPath;
}

Dvfs::DatPath Dvfs::DatPath::operator/(const char* subPath) const {
    return operator/(std::string(subPath));
}

Dvfs::DatPath Dvfs::DatPath::operator/(const Dvfs::DatPath& subPath) const {
    // By using two DatPaths we can guarantee that both sides are sanitised, so avoid extra sanitise by creating
    // an empty DatPath and setting its path manually
    DatPath newPath;
    // If this path/sub-path is empty then we have to make sure we don't append a "/" at the beginning/end
    newPath.path = empty()
            ? subPath.path
            : subPath.empty()
                    ? path
                    : (path + "/" + subPath.path);
    return newPath;
}

Dvfs::DatPath Dvfs::DatPath::getRoot() const {
    if (empty()) return {};
    return {path.substr(0, path.find_first_of('/'))};
}

size_t Dvfs::DatPath::depth() const {
    if (path.empty()) return 0;
    return std::count(path.begin(), path.end(), '/') + 1;
}

bool Dvfs::DatPath::empty() const {
    return path.empty();
}

std::vector<std::string_view> Dvfs::DatPath::split() const {
    auto parts = path | std::views::split('/')
            | std::views::filter([](auto rng) {
                return !rng.empty();
            })
            | std::views::transform([](auto rng) {
                  return std::string_view(rng.data(), rng.size());
            });
     return {parts.begin(), parts.end()};
}
