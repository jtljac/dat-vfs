//
// Created by jacob on 28/06/23.
//

#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include <cstring>
#include <algorithm>
#include <numeric>

#include "dat-path.h"
#include "dat-vfs-file.h"
#include "dat-vfs-file-inserter.h"

namespace DVFS {
    /**
     * A root or folder node in the Virtual File System
     */
    class DatVFS {
        std::unordered_map<std::string, DatVFS*> folders;
        std::unordered_map<std::string, IDVFSFile*> files;

    public:
        /**
         * Create a root node
         */
        DatVFS() {
            folders["."] = this;
            // Lacking a parent, set the parent to itself
            folders[".."] = this;
        }

        /**
         * Create a folder node
         * @param parent The parent of this folder node
         */
        DatVFS(DatVFS* parent) {
            folders["."] = this;
            folders[".."] = parent;
        }

        ~DatVFS() {
            for (auto& folder: folders) {
                delete folder.second;
            }
            folders.clear();

            for (auto& file: files) {
                delete file.second;
            }
            files.clear();
        }

        // Folder management
        /**
         * Create a folder in the VFS
         * @param path The path to the folder
         * @param recursive Whether to create parent directories as needed
         * @return The newly created node, or nullptr if the folder couldn't be created
         */
        DatVFS* createFolder(const DatPath& path, bool recursive = false) {
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

        // Mounting
        /**
         * Mount a DVFSFile on the VFS
         * @param path The path to mount the file at
         * @param dvfsFile The file to mount
         * @param createFolders Whether to create parent directories as needed
         * @return true if successful
         */
        bool mountFile(const DatPath& path, IDVFSFile* dvfsFile, bool createFolders = false) {
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

        /**
         * Mount multiple files on the VFS using an inserter
         * @param basePath The starting path to mount the files on
         * @param inserter A DVFSFileInserter defining what files to mount
         * @param createFolders Whether to create parent folders for the basePath and any paths for files being mounted
         * @return the number of files mounted
         */
        int mountFiles(const DatPath& basePath, const IDVFSFileInserter& inserter, bool createFolders = false) {
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

        // Unmount
        /**
         * Unmount a file from the VFS and delete it's DVFSFile entry
         * @param path The path to the file to unmount
         * @return True if successful
         */
        bool unmountFile(const DatPath& path) {
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

        /**
         * Remove a folder from the VFS, deleting it and all files and folders contained within
         * @param path The path to the folder to remove
         * @return True if successful
         */
        bool removeFolder(const DatPath& path) {
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

        // File Access
        /**
         * Get a file inside the VFS
         * @param path The path to the file
         * @return A pointer to the file, or nullptr if the file doesn't exist
         */
        IDVFSFile* getFile(const DatPath& path) {
            if (path.depth() > 1) {
                DatVFS* folder = getFolder(path.getRoot());
                if (folder == nullptr) return nullptr;

                return folder->getFile(path.increment());
            }

            auto it = files.find((std::string) path);
            return it != files.end() ? it->second : nullptr;
        }

        /**
         * Get a folder inside the VFS
         * @param path The path to the folder
         * @return A pointer to the folder, or nullptr if the folder doesn't exist
         */
        DatVFS* getFolder(const DatPath& path) {
            if (path.depth() > 1) {
                DatVFS* folder = getFolder(path.getRoot());
                if (folder == nullptr) return nullptr;

                return folder->getFolder(path.increment());
            }

            auto it = folders.find((std::string) path);
            return it != folders.end() ? it->second : nullptr;
        }

        // Util
        /**
         * Check if a file or folder exists
         * @param path The path to the file/folder
         * @return positive if a file, negative if a folder, 0 for doesn't exist
         */
        int exists(const DatPath& path) {
            if (path.depth() > 1) {
                DatVFS* folder = getFolder(path.getRoot());
                if (folder == nullptr) return false;
                else return folder->exists(path.increment());
            }

            // Avoids branching, assumes that there will never be a folder and a file with the same name
            return files.count((std::string) path.getRoot()) + (folders.count((std::string) path.getRoot()) * -1);
        }

        /**
         * Check if the given folder is empty
         * @param path The path to the folder
         * @return true if the given folder contains no files or folders
         */
         bool empty(const DatPath& path = DatPath()) {
            if (path.depth() > 0) {
                DatVFS* folder = getFolder(path.getRoot());
                if (folder == nullptr) return false;
                else return folder->empty(path.increment());
            }

            return folders.empty() && files.empty();
         }

         /**
          * Check if this folder is the root of the VFS
          * @return true if this folder is the roof of the VFS
          */
         bool isRoot(const std::filesystem::path& test) {
             return getFolder("..") == this;
         }

        /**
         * List the files in a directory
         * @param path The path to the folder to list (empty for the current folder)
         * @return A vector containing the names of all the files at the path
         */
        std::vector<std::string> listFiles(const DatPath& path = DatPath()) {
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

        /**
         * List the folders in a directory
         * @param path The path to the folder to list (empty for the current folder)
         * @return A vector containing the names of all the folders at the path
         */
        std::vector<std::string> listFolders(const DatPath& path = DatPath()) {
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

        /**
         * Remove folders from the given directory if they're empty
         * @param path The path to the folder start pruning from (empty for the current folder)
         * @param recursive Whether to also prune sub directories of the given directory
         * @return The number of folders deleted
         */
        int prune(const DatPath& path = DatPath(), bool recursive = false) {
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

        // Count Files (recursive, filter)
        /**
         * Count the number of files that match the filter in the given folder
         * <br>
         * The default filter matches all files
         * @param path The path to the folder to start counting from, empty for the current folder
         * @param recursive Whether to count files in subfolders too
         * @param predicate The filter that decides which files to count
         * @return The number of files that
         */
        int countFiles(const DatPath& path = DatPath(),
                       bool recursive = false,
                       const std::function<bool(const std::string&, IDVFSFile*)>& predicate = [](const std::string&, IDVFSFile*){return true;}) {
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

        /**
         * Count the number of folders that match the filter in the given folder
         * <br>
         * The default filter matches all folders
         * <br>
         * When recursive is true, the filter will not prevent searching inside folders that do not match
         * @param path The path to the folder to start counting from, empty for the current folder
         * @param recursive Whether to count folders in subfolders too
         * @param predicate The filter that decides which folders to count
         * @return The number of files that
         */
        int countFolders(const DatPath& path = DatPath(),
                       bool recursive = false,
                       const std::function<bool(const std::string&, DatVFS*)>& predicate = [](const std::string&, DatVFS*){return true;}) {
            if (path.depth() > 0) {
                DatVFS* folder = getFolder(path.getRoot());
                if (folder == nullptr) return {};
                else return folder->countFolders(path.increment(), recursive, predicate);
            }

            int count = std::accumulate(folders.begin(), folders.end(), 0, [&path, recursive, &predicate](int acc, const auto& pair){
                if (recursive) acc += pair.second->countFolders(path, recursive, predicate);
                return predicate(pair.first, pair.second) ? acc + 1 : acc;
            });

            return count;
        }

        /**
         * Generate a string displaying the structure of the VFS
         * @param depth The current depth, used for calculating how to display the file/folder in the tree
         * @return A string representing the structure of the VFS
         */
        std::string tree(const std::string& prefix = "") const {
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
    };
}