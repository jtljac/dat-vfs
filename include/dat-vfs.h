//
// Created by jacob on 28/06/23.
//

#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include <cstring>
#include <algorithm>

#include "dat-path.h"
#include "dat-vfs-file.h"
#include "dat-vfs-file-inserter.h"

namespace DatVFS {
    /**
     * Some exceptions that can be returned from operations
     */
    enum class DatVFSException {
        /** The file or folder already exists */
        EXISTS,
        /** The file or folder was not found */
        NOTFOUND
    };

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
        int mountFiles(const DatPath& basePath, IDVFSFileInserter* inserter, bool createFolders = false) {
            if (basePath.depth() > 1) {
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
            for (const auto& [path, idvfsFile]: inserter->getAllFiles()) {
                if (mountFile(path, idvfsFile, createFolders))++count;
                else {
                    inserter->handleInsertFailure(path, idvfsFile);
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
         * Remove a folder from the VFS, deleting all files and folders contained within
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

        // Get Files (recursive, filter)

        // Util
        /**
         * Check if a file or folder exists
         * @param path
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
        // List Files
        // List Folders
        // Size
        // Prune
        // Count Files (recursive, filter)
        // Count Folders (recursive, filter)
        // Tree

    };
}