#pragma once

#include <string>
#include <vector>
#include <span>
#include <unordered_map>
#include <functional>

#include "DatPath.h"
#include "DatVfsFile.h"
#include "DatVfsFileInserter.h"

namespace Dvfs {
    /**
     * A root or directory node in the Virtual File System
     */
    class DatVFS {
        std::unordered_map<std::string, DatVFS*> directories;
        std::unordered_map<std::string, IDvfsFile*> files;

        // Directory management
        /**
         * Create a directory in the VFS
         * @param path The path to the directory
         * @param recursive Whether to create parent directories as needed
         * @return The newly created node, or nullptr if the directory couldn't be created
         */
        DatVFS* createDirectory(std::span<std::string_view> path, bool recursive = false);

        // Mounting
        /**
         * Mount a DvfsFile on the VFS
         * @param path The path to mount the file at
         * @param dvfsFile The file to mount
         * @param createDirectories Whether to create parent directories as needed
         * @return true if successful
         */
        bool mountFile(std::span<std::string_view> path, IDvfsFile* dvfsFile, bool createDirectories = false);

        /**
         * Mount multiple files on the VFS using an inserter
         * @param basePath The starting path to mount the files on
         * @param inserter A DvfsFileInserter defining what files to mount
         * @param createDirectories Whether to create parent directories for the basePath and any paths for files being mounted
         * @return the number of files mounted
         */
        int mountFiles(std::span<std::string_view> path, const IDvfsFileInserter& inserter, bool createDirectories = false);

        // Unmount
        /**
         * Unmount a file from the VFS
         * @param path The path to the file to unmount
         * @param deleteDvfsFile Whether to delete the DvfsFile entry after unmounting it, defaults to true.
         * <br>
         * <b>Warning, If this is set to false, then failing to handle the DvfsFile will result in a memory leak!<b>
         * @return True if successful
         */
        bool unmountFile(std::span<std::string_view> path, bool deleteDvfsFile = true);

        /**
         * Remove a directory from the VFS, deleting it and all files and directories contained within
         * @param path The path to the directory to remove
         * @return True if successful
         */
        bool removeDirectory(std::span<std::string_view> path);

        // File Access
        /**
         * Get a file inside the VFS
         * @param path The path to the file
         * @return A pointer to the file, or nullptr if the file doesn't exist
         */
        IDvfsFile* getFile(std::span<std::string_view> path) const;

        /**
         * Get a directory inside the VFS
         * @param path The path to the directory
         * @return A pointer to the directory, or nullptr if the directory doesn't exist
         */
        DatVFS* getDirectory(std::span<std::string_view> path) const;

        // Util
        /**
         * Check if a file or directory exists
         * @param path The path to the file/directory
         * @return positive if a file, negative if a directory, 0 for doesn't exist
         */
        int exists(std::span<std::string_view> path) const;

        /**
         * Check if the given directory is empty
         * @param path The path to the directory
         * @return true if the given directory contains no files or directories
         */
         bool empty(std::span<std::string_view> path) const;

        // TODO:
        /**
         * List the files in a directory
         * @param path The path to the directory to list (empty for the current directory)
         * @return A vector containing the names of all the files at the path
         */
        std::vector<std::string> listFiles(std::span<std::string_view> path) const;

        /**
         * List the directories in a directory
         * @param path The path to the directory to list (empty for the current directory)
         * @return A vector containing the names of all the directories at the path
         */
        std::vector<std::string> listDirectories(std::span<std::string_view> path) const;

        /**
         * Remove directories from the given directory if they're empty
         * @param path The path to the directory start pruning from (empty for the current directory)
         * @param recursive Whether to also prune subdirectories of the given directory
         * @return The number of directories deleted
         */
        int prune(std::span<std::string_view> path, bool recursive = false);

        /**
         * Count the number of files that match the filter in the given directory
         * <br>
         * The default filter matches all files
         * @param path The path to the directory to start counting from, empty for the current directory
         * @param recursive Whether to count files in subdirectories too
         * @param predicate The filter that decides which files to count
         * @return The number of files that
         */
        int countFiles(std::span<std::string_view> path,
                       bool recursive = false,
                       const std::function<bool(const std::string&, IDvfsFile*)>& predicate = [](const std::string&, IDvfsFile*){return true;}) const;

        /**
         * Count the number of directories that match the filter in the given directory
         * <br>
         * The default filter matches all directories
         * <br>
         * When recursive is true, the filter will not prevent searching inside directories that do not match
         * @param path The path to the directory to start counting from, empty for the current directory
         * @param recursive Whether to count directories in subdirectories too
         * @param predicate The filter that decides which directories to count
         * @return The number of files that
         */
        int countDirectories(std::span<std::string_view> path,
                       bool recursive = false,
                       const std::function<bool(const std::string&, DatVFS*)>& predicate = [](const std::string&, DatVFS*){return true;}) const;

    public:
        /**
         * Create a root node
         */
        DatVFS();

        /**
         * Create a directory node
         * @param parent The parent of this directory node
         */
        explicit DatVFS(DatVFS* parent);

        ~DatVFS();

        // Directory management
        /**
         * Create a directory in the VFS
         * @param path The path to the directory
         * @param recursive Whether to create parent directories as needed
         * @return The newly created node, or nullptr if the directory couldn't be created
         */
        DatVFS* createDirectory(const DatPath& path, bool recursive = false);

        // Mounting
        /**
         * Mount a DvfsFile on the VFS
         * @param path The path to mount the file at
         * @param dvfsFile The file to mount
         * @param createDirectories Whether to create parent directories as needed
         * @return true if successful
         */
        bool mountFile(const DatPath& path, IDvfsFile* dvfsFile, bool createDirectories = false);

        /**
         * Mount multiple files on the VFS using an inserter
         * @param basePath The starting path to mount the files on
         * @param inserter A DvfsFileInserter defining what files to mount
         * @param createDirectories Whether to create parent directories for the basePath and any paths for files being mounted
         * @return the number of files mounted
         */
        int mountFiles(const DatPath& basePath, const IDvfsFileInserter& inserter, bool createDirectories = false);

        // Unmount
        /**
         * Unmount a file from the VFS
         * @param path The path to the file to unmount
         * @param deleteDvfsFile Whether to delete the DvfsFile entry after unmounting it, defaults to true.
         * <br>
         * <b>Warning, If this is set to false, then failing to handle the DvfsFile will result in a memory leak!<b>
         * @return True if successful
         */
        bool unmountFile(const DatPath& path, bool deleteDvfsFile = true);

        /**
         * Remove a directory from the VFS, deleting it and all files and directories contained within
         * @param path The path to the directory to remove
         * @return True if successful
         */
        bool removeDirectory(const DatPath& path);

        // File Access
        /**
         * Get a file inside the VFS
         * @param path The path to the file
         * @return A pointer to the file, or nullptr if the file doesn't exist
         */
        IDvfsFile* getFile(const DatPath& path) const;

        /**
         * Get a directory inside the VFS
         * @param path The path to the directory
         * @return A pointer to the directory, or nullptr if the directory doesn't exist
         */
        DatVFS* getDirectory(const DatPath& path) const;

        // Util
        /**
         * Check if a file or directory exists
         * @param path The path to the file/directory
         * @return positive if a file, negative if a directory, 0 for doesn't exist
         */
        int exists(const DatPath& path) const;

        /**
         * Check if the given directory is empty
         * @param path The path to the directory
         * @return true if the given directory contains no files or directories
         */
         bool empty(const DatPath& path = DatPath()) const;

         /**
          * Check if this directory is the root of the VFS
          * @return true if this directory is the roof of the VFS
          */
         bool isRoot() const;

        /**
         * List the files in a directory
         * @param path The path to the directory to list (empty for the current directory)
         * @return A vector containing the names of all the files at the path
         */
        std::vector<std::string> listFiles(const DatPath& path = DatPath()) const;

        /**
         * List the directories in a directory
         * @param path The path to the directory to list (empty for the current directory)
         * @return A vector containing the names of all the directories at the path
         */
        std::vector<std::string> listDirectories(const DatPath& path = DatPath()) const;

        /**
         * Remove directories from the given directory if they're empty
         * @param path The path to the directory start pruning from (empty for the current directory)
         * @param recursive Whether to also prune subdirectories of the given directory
         * @return The number of directories deleted
         */
        int prune(const DatPath& path = DatPath(), bool recursive = false);

        /**
         * Count the number of files that match the filter in the given directory
         * <br>
         * The default filter matches all files
         * @param path The path to the directory to start counting from, empty for the current directory
         * @param recursive Whether to count files in subdirectories too
         * @param predicate The filter that decides which files to count
         * @return The number of files that
         */
        int countFiles(const DatPath& path = DatPath(),
                       bool recursive = false,
                       const std::function<bool(const std::string&, IDvfsFile*)>& predicate = [](const std::string&, IDvfsFile*){return true;}) const;

        /**
         * Count the number of directories that match the filter in the given directory
         * <br>
         * The default filter matches all directories
         * <br>
         * When recursive is true, the filter will not prevent searching inside directories that do not match
         * @param path The path to the directory to start counting from, empty for the current directory
         * @param recursive Whether to count directories in subdirectories too
         * @param predicate The filter that decides which directories to count
         * @return The number of files that
         */
        int countDirectories(const DatPath& path = DatPath(),
                       bool recursive = false,
                       const std::function<bool(const std::string&, DatVFS*)>& predicate = [](const std::string&, DatVFS*){return true;}) const;

        /**
         * Generate a string displaying the structure of the VFS
         * @param prefix The current depth, used for calculating how to display the file/directory in the tree
         * @return A string representing the structure of the VFS
         */
        std::string tree(const std::string& prefix = "") const;
    };
}