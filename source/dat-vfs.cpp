//
// Created by jacob on 28/06/23.
//

# include "../include/dat-vfs.h"

#include <algorithm>
#include <numeric>

DVFS::DatVFS::DatVFS() {
    folders["."] = this;
    // Lacking a parent, set the parent to itself
    folders[".."] = this;
}

DVFS::DatVFS::DatVFS(DVFS::DatVFS* parent) {
    folders["."] = this;
    folders[".."] = parent;
}

DVFS::DatVFS::~DatVFS() {
    for (auto& folder: folders) {
        delete folder.second;
    }
    folders.clear();

    for (auto& file: files) {
        delete file.second;
    }
    files.clear();
}

DVFS::DatVFS* DVFS::DatVFS::createFolder(const DVFS::DatPath& path, bool recursive) {
    if (path.empty()) return nullptr;

    if (path.depth() > 1) {
        DatVFS* folder = getFolder(path.getRoot());
        if (folder == nullptr) {
            if (!recursive) return nullptr;

            // Call create folder for the validation
            folder = createFolder(path.getRoot());
            if (folder == nullptr) return nullptr;
        }

        return folder->createFolder(path.increment());
    }

    if (exists(path)) return nullptr;

    DatVFS* folder = new DatVFS(this);
    folders.emplace((std::string) path.getRoot(), folder);
    return folder;
}

bool DVFS::DatVFS::mountFile(const DVFS::DatPath& path, DVFS::IDVFSFile* dvfsFile, bool createFolders) {
    if (path.depth() > 1) {
        DatVFS* folder = getFolder(path.getRoot());
        if (folder == nullptr) {
            if (!createFolders) return false;

            // Call create folder for the validation
            folder = createFolder(path.getRoot());
            if (folder == nullptr) return false;
        }

        return folder->mountFile(path.increment(), dvfsFile, createFolders);
    }

    if (exists(path)) return false;

    files.emplace((std::string) path.getRoot(), dvfsFile);
    return true;
}

int DVFS::DatVFS::mountFiles(const DVFS::DatPath& basePath, const DVFS::IDVFSFileInserter& inserter, bool createFolders) {
    if (basePath.depth() > 0) {
        DatVFS* folder = getFolder(basePath.getRoot());
        if (folder == nullptr) {
            if (!createFolders) return 0;

            // Call create folder for the validation
            folder = createFolder(basePath.getRoot());
            if (folder == nullptr) return 0;
        }

        return folder->mountFiles(basePath.increment(), inserter, createFolders);
    }

    int count = 0;
    for (const auto& [path, idvfsFile]: inserter.getAllFiles()) {
        if (mountFile(path, idvfsFile, createFolders)) ++count;
        else {
            inserter.handleInsertFailure(path, idvfsFile);
        }
    }

    return count;
}

bool DVFS::DatVFS::unmountFile(const DVFS::DatPath& path) {
    if (path.depth() > 1) {
        DatVFS* folder = getFolder(path.getRoot());
        if (folder == nullptr) return false;

        return folder->unmountFile(path.increment());
    }

    IDVFSFile* idvfsFile = getFile(path);

    if (idvfsFile == nullptr) return false;

    files.erase((std::string) path);
    delete idvfsFile;
    return true;
}

bool DVFS::DatVFS::removeFolder(const DVFS::DatPath& path) {
    DatVFS* folder = getFolder(path);
    if (folder == nullptr) return false;

    if (path.depth() > 1) {
        return folder->removeFolder(path.increment());
    }

    folders.erase((std::string) path);

    // File and subfolder deletion is handled by the destructor
    delete folder;
    return true;
}

DVFS::IDVFSFile* DVFS::DatVFS::getFile(const DVFS::DatPath& path) const {
    if (path.depth() > 1) {
        DatVFS* folder = getFolder(path.getRoot());
        if (folder == nullptr) return nullptr;

        return folder->getFile(path.increment());
    }

    auto it = files.find((std::string) path);
    return it != files.end() ? it->second : nullptr;
}

DVFS::DatVFS* DVFS::DatVFS::getFolder(const DVFS::DatPath& path) const {
    if (path.depth() > 1) {
        DatVFS* folder = getFolder(path.getRoot());
        if (folder == nullptr) return nullptr;

        return folder->getFolder(path.increment());
    }

    auto it = folders.find((std::string) path);
    return it != folders.end() ? it->second : nullptr;
}

int DVFS::DatVFS::exists(const DVFS::DatPath& path) const {
    if (path.depth() > 1) {
        DatVFS* folder = getFolder(path.getRoot());
        if (folder == nullptr) return false;
        else return folder->exists(path.increment());
    }

    // Avoids branching, assumes that there will never be a folder and a file with the same name
    return files.count((std::string) path.getRoot()) + (folders.count((std::string) path.getRoot()) * -1);
}

bool DVFS::DatVFS::empty(const DVFS::DatPath& path) const {
    if (path.depth() > 0) {
        DatVFS* folder = getFolder(path.getRoot());
        if (folder == nullptr) return false;
        else return folder->empty(path.increment());
    }

    // There's always "." and ".."
    return folders.size() <= 2 && files.empty();
}

bool DVFS::DatVFS::isRoot() const {
    return getFolder("..") == this;
}

std::vector<std::string> DVFS::DatVFS::listFiles(const DVFS::DatPath& path) const {
    if (path.depth() > 0) {
        DatVFS* folder = getFolder(path.getRoot());
        if (folder == nullptr) return {};
        else return folder->listFiles(path.increment());
    }

    std::vector<std::string> fileNames;
    fileNames.reserve(files.size());
    std::transform(files.begin(), files.end(), std::back_inserter(fileNames), [](const auto& pair){return pair.first;});

    return fileNames;
}

std::vector<std::string> DVFS::DatVFS::listFolders(const DVFS::DatPath& path) const {
    if (path.depth() > 0) {
        DatVFS* folder = getFolder(path.getRoot());
        if (folder == nullptr) return {};
        else return folder->listFiles(path.increment());
    }

    std::vector<std::string> folderNames;
    folderNames.reserve(folders.size());
    std::transform(folders.begin(), folders.end(), std::back_inserter(folderNames), [](const auto& pair){return pair.first;});

    return folderNames;
}

int DVFS::DatVFS::prune(const DVFS::DatPath& path, bool recursive) {
    if (path.depth() > 0) {
        DatVFS* folder = getFolder(path.getRoot());
        if (folder == nullptr) return {};
        else return folder->prune(path.increment(), recursive);
    }

    int count = 0;
    auto it = folders.begin();
    while (it != folders.end()) {
        DatVFS* folder = it->second;

        // Do recursive first, so we can prune a folder that becomes empty after pruning
        if (recursive) {
            count += folder->prune(path, recursive);
        }

        if (folder->empty()) {
            folders.erase(it++);
            ++count;
        }
    }

    return count;
}

int DVFS::DatVFS::countFiles(const DVFS::DatPath& path, bool recursive,
                             const std::function<bool(const std::string&, IDVFSFile*)>& predicate) const {
    if (path.depth() > 0) {
        DatVFS* folder = getFolder(path.getRoot());
        if (folder == nullptr) return {};
        else return folder->countFiles(path.increment(), recursive, predicate);
    }

    int count = std::accumulate(files.begin(), files.end(), 0, [&predicate](int acc, const auto& pair){
        return predicate(pair.first, pair.second) ? acc + 1 : acc;
    });

    if (recursive) {
        count += std::accumulate(folders.begin(), folders.end(), 0, [&predicate, &path](int acc, const auto& pair) {
            return acc + pair.second->countFiles(path, true, predicate);
        });
    };

    return count;
}

int DVFS::DatVFS::countFolders(const DVFS::DatPath& path, bool recursive,
                               const std::function<bool(const std::string&, DatVFS*)>& predicate) const {
    if (path.depth() > 0) {
        DatVFS* folder = getFolder(path.getRoot());
        if (folder == nullptr) return {};
        else return folder->countFolders(path.increment(), recursive, predicate);
    }

    // Start at negative 2 to account for "." and ".."
    int count = -2 + std::accumulate(folders.begin(), folders.end(), 0, [&path, recursive, &predicate](int acc, const auto& pair){
        if (recursive) acc += pair.second->countFolders(path, recursive, predicate);
        return predicate(pair.first, pair.second) ? acc + 1 : acc;
    });

    return count;
}

std::string DVFS::DatVFS::tree(const std::string& prefix) const {
    std::stringstream stream;

    bool noFiles = files.empty();
    {
        auto it = folders.begin();
        while (it != folders.end()) {
            const std::string& name = it->first;
            const DatVFS* folder = it->second;
            bool end = std::next(it) == folders.end() && noFiles;

            stream << prefix << (end ? "└── " : "├── ") << name << std::endl;

            if (name == "." || name == "..") {
                ++it;
                continue;
            }

            stream << folder->tree(prefix + (end ? "    " : "│   "));
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
