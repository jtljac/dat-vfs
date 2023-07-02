//
// Created by jacob on 28/06/23.
//

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

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
        DatVFS();

        /**
         * Create a folder node
         * @param parent The parent of this folder node
         */
        explicit DatVFS(DatVFS* parent);

        ~DatVFS();

        // Folder management
        /**
         * Create a folder in the VFS
         * @param path The path to the folder
         * @param recursive Whether to create parent directories as needed
         * @return The newly created node, or nullptr if the folder couldn't be created
         */
        DatVFS* createFolder(const DatPath& path, bool recursive = false);

        // Mounting
        /**
         * Mount a DVFSFile on the VFS
         * @param path The path to mount the file at
         * @param dvfsFile The file to mount
         * @param createFolders Whether to create parent directories as needed
         * @return true if successful
         */
        bool mountFile(const DatPath& path, IDVFSFile* dvfsFile, bool createFolders = false);

        /**
         * Mount multiple files on the VFS using an inserter
         * @param basePath The starting path to mount the files on
         * @param inserter A DVFSFileInserter defining what files to mount
         * @param createFolders Whether to create parent folders for the basePath and any paths for files being mounted
         * @return the number of files mounted
         */
        int mountFiles(const DatPath& basePath, const IDVFSFileInserter& inserter, bool createFolders = false);

        // Unmount
        /**
         * Unmount a file from the VFS
         * @param path The path to the file to unmount
         * @param deleteDVFSFile Whether to delete the DVFSFile entry after unmounting it, defaults to true.
         * <br>
         * <b>Warning, If this is set to false, then failing to handle the DVFSFile will result in a memory leak!<b>
         * @return True if successful
         */
        bool unmountFile(const DatPath& path, bool deleteDVFSFile = true);

        /**
         * Remove a folder from the VFS, deleting it and all files and folders contained within
         * @param path The path to the folder to remove
         * @return True if successful
         */
        bool removeFolder(const DatPath& path);

        // File Access
        /**
         * Get a file inside the VFS
         * @param path The path to the file
         * @return A pointer to the file, or nullptr if the file doesn't exist
         */
        IDVFSFile* getFile(const DatPath& path) const;

        /**
         * Get a folder inside the VFS
         * @param path The path to the folder
         * @return A pointer to the folder, or nullptr if the folder doesn't exist
         */
        DatVFS* getFolder(const DatPath& path) const;

        // Util
        /**
         * Check if a file or folder exists
         * @param path The path to the file/folder
         * @return positive if a file, negative if a folder, 0 for doesn't exist
         */
        int exists(const DatPath& path) const;

        /**
         * Check if the given folder is empty
         * @param path The path to the folder
         * @return true if the given folder contains no files or folders
         */
         bool empty(const DatPath& path = DatPath()) const;

         /**
          * Check if this folder is the root of the VFS
          * @return true if this folder is the roof of the VFS
          */
         bool isRoot() const;

        /**
         * List the files in a directory
         * @param path The path to the folder to list (empty for the current folder)
         * @return A vector containing the names of all the files at the path
         */
        std::vector<std::string> listFiles(const DatPath& path = DatPath()) const;

        /**
         * List the folders in a directory
         * @param path The path to the folder to list (empty for the current folder)
         * @return A vector containing the names of all the folders at the path
         */
        std::vector<std::string> listFolders(const DatPath& path = DatPath()) const;

        /**
         * Remove folders from the given directory if they're empty
         * @param path The path to the folder start pruning from (empty for the current folder)
         * @param recursive Whether to also prune sub directories of the given directory
         * @return The number of folders deleted
         */
        int prune(const DatPath& path = DatPath(), bool recursive = false);

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
                       const std::function<bool(const std::string&, IDVFSFile*)>& predicate = [](const std::string&, IDVFSFile*){return true;}) const;

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
                       const std::function<bool(const std::string&, DatVFS*)>& predicate = [](const std::string&, DatVFS*){return true;}) const;

        /**
         * Generate a string displaying the structure of the VFS
         * @param depth The current depth, used for calculating how to display the file/folder in the tree
         * @return A string representing the structure of the VFS
         */
        std::string tree(const std::string& prefix = "") const;
    };
}