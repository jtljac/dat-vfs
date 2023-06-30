//
// Created by jacob on 29/06/23.
//

#include "../include/dat-path.h"

#include <algorithm>
#include <cassert>

DVFS::DatPath::DatPath(std::string path) {
    if (path.empty()) {
        return;
    }

    // Ensure backslashes aren't used
    assert(path.find('\\') == std::string::npos);

    std::string::size_type startIndex = path.find_first_not_of('/');
    std::string::size_type endIndex = path.find_last_not_of('/');

    this->path = path.substr(startIndex,
                             endIndex != std::string::npos
                             ? endIndex + 1 - startIndex
                             : std::string::npos);
}

DVFS::DatPath::operator std::string() const {
    return path;
}

bool DVFS::DatPath::operator==(const DatPath& rh) const {
    return path == rh.path;
}

DVFS::DatPath DVFS::DatPath::increment(size_t levels) const {
    std::string::size_type index = 0;
    while (levels-- > 0) {
        std::string::size_type newIndex = path.find_first_of('/', index);
        if (newIndex == std::string::npos) return {};
        index = newIndex + 1;
    }

    return {path.substr(index)};
}

DVFS::DatPath DVFS::DatPath::getRoot() const {
    if (empty()) return {};
    return {path.substr(0, path.find_first_of('/'))};
}

size_t DVFS::DatPath::depth() const {
    if (path.empty()) return 0;
    return std::count(path.begin(), path.end(), '/') + 1;
}

bool DVFS::DatPath::empty() const {
    return path.empty();
}
