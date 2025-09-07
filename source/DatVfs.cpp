# include "../include/DatVfs.h"

#include <algorithm>
#include <numeric>
#include <ranges>
#include <oneapi/tbb/detail/_range_common.h>

Dvfs::DatVFS::DatVFS() {
    directories["."] = this;
    // Lacking a parent, set the parent to itself
    directories[".."] = this;
}

Dvfs::DatVFS::DatVFS(Dvfs::DatVFS* parent) {
    directories["."] = this;
    directories[".."] = parent;
}

Dvfs::DatVFS::~DatVFS() {
    for (auto& directory: directories) {
        delete directory.second;
    }
    directories.clear();

    for (auto& file: files) {
        // Only delete if we know this is the only reference to the file
        if (file.second->decrementReferences() <= 0) delete file.second;
    }
    files.clear();
}

Dvfs::DatVFS* Dvfs::DatVFS::createDirectory(const std::span<std::string_view> path, const bool recursive) {
    if (path.empty()) return nullptr;

    if (path.size() > 1) {
        DatVFS* directory = getDirectory(path.subspan(0, 1));
        if (directory == nullptr) {
            if (!recursive) return nullptr;

            // Call create directory for the validation
            directory = createDirectory(path.subspan(0, 1));
            if (directory == nullptr) return nullptr;
        }

        return directory->createDirectory(path.subspan(1, path.size() - 1), recursive);
    }

    if (exists(path)) return nullptr;

    DatVFS* directory = new DatVFS(this);
    directories.emplace(path[0], directory);
    return directory;
}

Dvfs::DatVFS* Dvfs::DatVFS::createDirectory(const DatPath& path, const bool recursive) {
    std::vector<std::string_view> paths = path.split();
    return createDirectory(std::span(paths), recursive);
}


bool Dvfs::DatVFS::mountFile(const std::span<std::string_view> path, IDvfsFile* dvfsFile, bool createDirectories) {
    if (path.size() > 1) {
        DatVFS* directory = getDirectory(path.subspan(0, 1));
        if (directory == nullptr) {
            if (!createDirectories) return false;

            // Call create directory for the validation
            directory = createDirectory(path.subspan(0, 1));
            if (directory == nullptr) return false;
        }

        return directory->mountFile(path.subspan(1, path.size() - 1), dvfsFile, createDirectories);
    }

    if (exists(path)) return false;

    files.emplace(path[0], dvfsFile);
    dvfsFile->incrementReferences();
    return true;
}


bool Dvfs::DatVFS::mountFile(const DatPath& path, IDvfsFile* dvfsFile, bool createDirectories) {
    std::vector<std::string_view> paths = path.split();
    return mountFile(std::span(paths), dvfsFile, createDirectories);
}

int Dvfs::DatVFS::mountFiles(const std::span<std::string_view> path, const IDvfsFileInserter& inserter, bool createDirectories) {
    if (!path.empty()) {
        DatVFS* directory = getDirectory(path.subspan(0, 1));
        if (directory == nullptr) {
            if (!createDirectories) return 0;

            // Call create directory for the validation
            directory = createDirectory(path.subspan(0, 1));
            if (directory == nullptr) return 0;
        }

        return directory->mountFiles(path.subspan(1, path.size() - 1), inserter, createDirectories);
    }

    int count = 0;
    for (const auto& [path, iDvfsFile]: inserter.getAllFiles()) {
        if (mountFile(path, iDvfsFile, createDirectories)) ++count;
        else {
            inserter.handleInsertFailure(path, iDvfsFile);
        }
    }

    return count;
}


int Dvfs::DatVFS::mountFiles(const DatPath& basePath, const IDvfsFileInserter& inserter, bool createDirectories) {
    std::vector<std::string_view> paths = basePath.split();
    return mountFiles(std::span(paths), inserter, createDirectories);
}

bool Dvfs::DatVFS::unmountFile(const std::span<std::string_view> path, const bool deleteDvfsFile) {
    if (path.size() > 1) {
        DatVFS* directory = getDirectory(path.subspan(0, 1));
        if (directory == nullptr) return false;

        return directory->unmountFile(path.subspan(1, path.size() - 1), deleteDvfsFile);
    }

    IDvfsFile* iDvfsFile = getFile(path);

    if (iDvfsFile == nullptr) return false;

    files.erase(static_cast<std::string>(path[0]));

    // Only delete if we know this is the only reference in the Dvfs
    if (iDvfsFile->decrementReferences() <= 0 && deleteDvfsFile) delete iDvfsFile;

    return true;
}


bool Dvfs::DatVFS::unmountFile(const DatPath& path, const bool deleteDvfsFile) {
    std::vector<std::string_view> paths = path.split();
    return unmountFile(std::span(paths), deleteDvfsFile);}

bool Dvfs::DatVFS::removeDirectory(const std::span<std::string_view> path) {
    DatVFS* directory = getDirectory(path);
    if (directory == nullptr) return false;

    if (path.size() > 1) {
        return directory->removeDirectory(path.subspan(1, path.size() - 1));
    }

    directories.erase(static_cast<std::string>(path[0]));

    // File and subdirectory deletion is handled by the destructor
    delete directory;
    return true;
}

bool Dvfs::DatVFS::removeDirectory(const DatPath& path) {
    std::vector<std::string_view> paths = path.split();
    return removeDirectory(std::span(paths));
}


Dvfs::IDvfsFile* Dvfs::DatVFS::getFile(const std::span<std::string_view> path) const {
    if (path.size() > 1) {
        DatVFS* directory = getDirectory(path.subspan(0, 1));
        if (directory == nullptr) return nullptr;

        return directory->getFile(path.subspan(1, path.size() - 1));
    }

    auto it = files.find(static_cast<std::string>(path[0]));
    return it != files.end() ? it->second : nullptr;
}

Dvfs::IDvfsFile* Dvfs::DatVFS::getFile(const DatPath& path) const {
    std::vector<std::string_view> paths = path.split();
    return getFile(std::span(paths));

}

Dvfs::DatVFS* Dvfs::DatVFS::getDirectory(const std::span<std::string_view> path) const {
    if (path.size() > 1) {
        DatVFS* directory = getDirectory(path.subspan(0, 1));
        if (directory == nullptr) return nullptr;

        return directory->getDirectory(path.subspan(1, path.size() - 1));
    }

    auto it = directories.find(static_cast<std::string>(path[0]));
    return it != directories.end() ? it->second : nullptr;
}

Dvfs::DatVFS* Dvfs::DatVFS::getDirectory(const DatPath& path) const {
    std::vector<std::string_view> paths = path.split();
    return getDirectory(std::span(paths));
}

int Dvfs::DatVFS::exists(const std::span<std::string_view> path) const {
    if (path.size() > 1) {
        DatVFS* directory = getDirectory(path.subspan(0, 1));
        if (directory == nullptr) return false;
        return directory->exists(path.subspan(1, path.size() - 1));
    }

    // Avoids branching, assumes that there will never be a directory and a file with the same name
    return files.count(static_cast<std::string>(path[0])) + (directories.count(static_cast<std::string>(path[0])) * -1);
}

int Dvfs::DatVFS::exists(const DatPath& path) const {
    std::vector<std::string_view> paths = path.split();
    return exists(std::span(paths));
}

bool Dvfs::DatVFS::empty(const std::span<std::string_view> path) const {
    if (!path.empty()) {
        const DatVFS* directory = getDirectory(path.subspan(0, 1));
        if (directory == nullptr) return false;
        return directory->empty(path.subspan(1, path.size() - 1));
    }

    // There's always "." and ".."
    return directories.size() <= 2 && files.empty();
}


bool Dvfs::DatVFS::empty(const DatPath& path) const {
    std::vector<std::string_view> paths = path.split();
    return empty(std::span(paths));
}

bool Dvfs::DatVFS::isRoot() const {
    return getDirectory("..") == this;
}

std::vector<std::string> Dvfs::DatVFS::listFiles(const std::span<std::string_view> path) const {
    if (!path.empty()) {
        DatVFS* directory = getDirectory(path.subspan(0, 1));
        if (directory == nullptr) return {};
        return directory->listFiles(path.subspan(1, path.size() - 1));
    }

    std::vector<std::string> fileNames;
    fileNames.reserve(files.size());
    std::ranges::transform(files, std::back_inserter(fileNames), [](const auto& pair){return pair.first;});

    return fileNames;
}

std::vector<std::string> Dvfs::DatVFS::listFiles(const DatPath& path) const {
    std::vector<std::string_view> paths = path.split();
    return listFiles(std::span(paths));

}

std::vector<std::string> Dvfs::DatVFS::listDirectories(const std::span<std::string_view> path) const {
    if (!path.empty()) {
        const DatVFS* directory = getDirectory(path.subspan(0, 1));
        if (directory == nullptr) return {};
        return directory->listFiles(path.subspan(1, path.size() - 1));
    }

    std::vector<std::string> directoryNames;
    directoryNames.reserve(directories.size());
    std::ranges::transform(directories, std::back_inserter(directoryNames), [](const auto& pair){return pair.first;});

    return directoryNames;
}

std::vector<std::string> Dvfs::DatVFS::listDirectories(const DatPath& path) const {
    std::vector<std::string_view> paths = path.split();
    return listDirectories(std::span(paths));
}

int Dvfs::DatVFS::prune(const std::span<std::string_view> path, const bool recursive) {
    if (!path.empty()) {
        DatVFS* directory = getDirectory(path.subspan(0, 1));
        if (directory == nullptr) return {};
        return directory->prune(path.subspan(1, path.size() - 1), recursive);
    }

    int count = 0;
    auto it = directories.begin();
    while (it != directories.end()) {
        // Skip Infinite recursion
        if (std::string name = it->first; name == "." || name == "..") {
            ++it;
            continue;
        }

        DatVFS* directory = it->second;

        // Do recursive first, so we can prune a directory that becomes empty after pruning
        if (recursive) {
            count += directory->prune(path, recursive);
        }

        if (directory->empty()) {
            directories.erase(it++);
            ++count;
        }
    }

    return count;
}

int Dvfs::DatVFS::prune(const DatPath& path, const bool recursive) {
    std::vector<std::string_view> paths = path.split();
    return prune(std::span(paths), recursive);
}

int Dvfs::DatVFS::countFiles(const std::span<std::string_view> path, bool recursive,
                             const std::function<bool(const std::string&, IDvfsFile*)>& predicate) const {
    if (!path.empty()) {
        DatVFS* directory = getDirectory(path.subspan(0, 1));
        if (directory == nullptr) return {};
        return directory->countFiles(path.subspan(0, path.size() - 1), recursive, predicate);
    }

    int count = std::accumulate(files.begin(), files.end(), 0, [&predicate](int acc, const auto& pair){
        return predicate(pair.first, pair.second) ? acc + 1 : acc;
    });

    if (recursive) {
        count += std::accumulate(directories.begin(), directories.end(), 0, [&predicate, &path](int acc, const auto& pair) {
            // Skip Infinite recursion
            if (pair.first == "." || pair.first == "..") return acc;
            return acc + pair.second->countFiles(path, true, predicate);
        });
    };

    return count;
}

int Dvfs::DatVFS::countFiles(const DatPath& path, bool recursive,
                             const std::function<bool(const std::string&, IDvfsFile*)>& predicate) const {
    std::vector<std::string_view> paths = path.split();
    return countFiles(std::span(paths), recursive, predicate);
}

int Dvfs::DatVFS::countDirectories(const std::span<std::string_view> path, bool recursive,
                               const std::function<bool(const std::string&, DatVFS*)>& predicate) const {
    if (!path.empty()) {
        DatVFS* directory = getDirectory(path.subspan(0, 1));
        if (directory == nullptr) return {};
        return directory->countDirectories(path.subspan(1, path.size() - 1), recursive, predicate);
    }

    int count = std::accumulate(directories.begin(), directories.end(), 0, [&path, recursive, &predicate](int acc, const auto& pair){
        // Skip Infinite recursion
        if (pair.first == "." || pair.first == "..") return acc;
        if (recursive) {
            acc += pair.second->countDirectories(path, recursive, predicate);
        }
        return predicate(pair.first, pair.second) ? acc + 1 : acc;
    });

    return count;
}

int Dvfs::DatVFS::countDirectories(const DatPath& path, const bool recursive,
                               const std::function<bool(const std::string&, DatVFS*)>& predicate) const {
    std::vector<std::string_view> paths = path.split();
    return countDirectories(std::span(paths), recursive, predicate);
}

std::string Dvfs::DatVFS::tree(const std::string& prefix) const {
    std::stringstream stream;

    bool noFiles = files.empty();
    {
        auto it = directories.begin();
        while (it != directories.end()) {
            const std::string& name = it->first;
            const DatVFS* directory = it->second;
            bool end = std::next(it) == directories.end() && noFiles;

            stream << prefix << (end ? "└── " : "├── ") << name << std::endl;

            // Skip infinite recursion
            if (name == "." || name == "..") {
                ++it;
                continue;
            }

            stream << directory->tree(prefix + (end ? "    " : "│   "));
            ++it;
        }
    }

    if (!noFiles) {
        auto it = files.begin();
        while (it != files.end()) {
            const std::string& name = it->first;
            bool end = std::next(it) == files.end();
            stream << prefix << (end ? "└── " : "├── ") << name << std::endl;
            ++it;
        }
    }

    return stream.str();
}
