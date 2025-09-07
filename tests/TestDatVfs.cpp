#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include <DatVfs.h>

using namespace Dvfs;

class MockDvfsFile : public IDvfsFile {
public:
    [[nodiscard]] uint64_t fileSize() const override {
        return 0;
    }

    [[nodiscard]] bool isValidFile() const override {
        return false;
    }

    bool getContent(char* buffer) const override {
        return false;
    }
};

class MockDvfsFileInserter : public IDvfsFileInserter {
public:
    [[nodiscard]] std::vector<pair> getAllFiles() const override {
        return {
                {"test", new MockDvfsFile},
                {"test2", new MockDvfsFile},
                {"test3", new MockDvfsFile},
                {"test4", new MockDvfsFile},
                {"directory/test", new MockDvfsFile},
                {"directory/test2", new MockDvfsFile},
                {"directory/test3", new MockDvfsFile},
                {"directory/test4", new MockDvfsFile},
                {"directory2/test", new MockDvfsFile},
                {"directory2/test2", new MockDvfsFile},
                {"directory2/test3", new MockDvfsFile},
                {"directory2/test4", new MockDvfsFile},
                {"directory2/directory/test", new MockDvfsFile},
                {"directory2/directory/test2", new MockDvfsFile},
                {"directory2/directory/test3", new MockDvfsFile},
                {"directory2/directory/test4", new MockDvfsFile},
        };
    }
};

TEST_CASE("DatVFS Empty VFS", "[DatVFS]") {
    DatVFS* vfs = new DatVFS;

    // Create directory
    SECTION("Create single directory") {
        DatPath path("test");
        REQUIRE(vfs->createDirectory(path) != nullptr);
        REQUIRE(vfs->exists(path));
    }

    SECTION("Create recursive directory") {
        DatPath path("test/recursive");
        REQUIRE(vfs->createDirectory(path, true) != nullptr);
        REQUIRE(vfs->exists(path) == -1);
    }

    SECTION("Create recursive directory wrong") {
        DatPath path("test/recursive");
        REQUIRE_FALSE(vfs->createDirectory(path, false) != nullptr);
        REQUIRE_FALSE(vfs->exists(path) == -1);
    }

    // Mount file
    SECTION("Mount File") {
        DatPath path("test");
        IDvfsFile* file = new MockDvfsFile;
        REQUIRE(vfs->mountFile(path, file));
        REQUIRE(vfs->exists(path) == 1);
    }


    SECTION("Mount File reference counting") {
        DatPath path("test");
        DatPath path2("test2");
        IDvfsFile* file = new MockDvfsFile;
        REQUIRE(file->getReferenceCount() == 0);
        REQUIRE(vfs->mountFile(path, file));
        REQUIRE(vfs->exists(path) == 1);
        REQUIRE(file->getReferenceCount() == 1);

        // Mounting in the second place
        REQUIRE(vfs->mountFile(path2, file));
        REQUIRE(vfs->exists(path2) == 1);
        REQUIRE(vfs->getFile(path) == vfs->getFile(path2));
        REQUIRE(file->getReferenceCount() == 2);

        // Unmounting
        REQUIRE(vfs->unmountFile(path2));
        REQUIRE(vfs->exists(path2) == 0);
        REQUIRE(file->getReferenceCount() == 1);
    }

    SECTION("Mount file Create Directories") {
        DatPath path("test/create/directories");
        REQUIRE(vfs->mountFile(path, new MockDvfsFile, true));
        REQUIRE(vfs->exists(path) == 1);
    }

    SECTION("Mount file Create Directories wrong") {
        DatPath path("test/create/directories");
        REQUIRE_FALSE(vfs->mountFile(path, new MockDvfsFile, false));
        REQUIRE_FALSE(vfs->exists(path) == 1);
    }

    SECTION("Mount files, create directories") {
        REQUIRE(vfs->mountFiles("", MockDvfsFileInserter(), true) == 16);

        REQUIRE(vfs->exists("test"));
        REQUIRE(vfs->exists("test2"));
        REQUIRE(vfs->exists("test3"));
        REQUIRE(vfs->exists("test4"));
        REQUIRE(vfs->exists("directory/test"));
        REQUIRE(vfs->exists("directory/test2"));
        REQUIRE(vfs->exists("directory/test3"));
        REQUIRE(vfs->exists("directory/test4"));
        REQUIRE(vfs->exists("directory2/test"));
        REQUIRE(vfs->exists("directory2/test2"));
        REQUIRE(vfs->exists("directory2/test3"));
        REQUIRE(vfs->exists("directory2/test4"));
        REQUIRE(vfs->exists("directory2/directory/test"));
        REQUIRE(vfs->exists("directory2/directory/test2"));
        REQUIRE(vfs->exists("directory2/directory/test3"));
        REQUIRE(vfs->exists("directory2/directory/test4"));
    }

    SECTION("Mount files, don't create directories") {
        REQUIRE(vfs->mountFiles("", MockDvfsFileInserter(), false) == 4);

        REQUIRE(vfs->exists("test"));
        REQUIRE(vfs->exists("test2"));
        REQUIRE(vfs->exists("test3"));
        REQUIRE(vfs->exists("test4"));
        REQUIRE_FALSE(vfs->exists("directory/test"));
        REQUIRE_FALSE(vfs->exists("directory/test2"));
        REQUIRE_FALSE(vfs->exists("directory/test3"));
        REQUIRE_FALSE(vfs->exists("directory/test4"));
        REQUIRE_FALSE(vfs->exists("directory2/test"));
        REQUIRE_FALSE(vfs->exists("directory2/test2"));
        REQUIRE_FALSE(vfs->exists("directory2/test3"));
        REQUIRE_FALSE(vfs->exists("directory2/test4"));
        REQUIRE_FALSE(vfs->exists("directory2/directory/test"));
        REQUIRE_FALSE(vfs->exists("directory2/directory/test2"));
        REQUIRE_FALSE(vfs->exists("directory2/directory/test3"));
        REQUIRE_FALSE(vfs->exists("directory2/directory/test4"));
    }

    // Empty
    SECTION("Root is empty") {
        REQUIRE(vfs->empty());
    }

    SECTION("Root contains file") {
        DatPath path("test");
        REQUIRE(vfs->mountFile(path, new MockDvfsFile));
        REQUIRE_FALSE(vfs->empty());
    }

    SECTION("Root contains directory") {
        DatPath path("test");
        REQUIRE(vfs->createDirectory(path));
        REQUIRE_FALSE(vfs->empty());
    }

    SECTION("Sub-directory is empty") {
        DatPath path("directory");
        vfs->createDirectory(path);

        REQUIRE(vfs->empty(path));
    }


    SECTION("Sub-directory contains file") {
        DatPath path("test");
        vfs->createDirectory(path);
        REQUIRE(vfs->mountFile(path/"file", new MockDvfsFile));

        REQUIRE_FALSE(vfs->empty(path));
    }

    SECTION("Sub-directory contains directory") {
        DatPath path("test");
        REQUIRE(vfs->createDirectory(path/"directory", true));
        REQUIRE_FALSE(vfs->empty());
    }

    // Is Root
    SECTION("Is Root in root") {
        REQUIRE(vfs->isRoot());
    }
}

TEST_CASE("DatVFS populated VFS", "[DatVFS]") {
    DatVFS* vfs = new DatVFS;

    vfs->mountFiles("", MockDvfsFileInserter(), true);

    // Mount file
    SECTION("Mount file already exists") {
        DatPath path("test");
        IDvfsFile* file = new MockDvfsFile;
        REQUIRE_FALSE(vfs->mountFile(path, file));

        // Check file hasn't replaced old file
        REQUIRE_FALSE(vfs->getFile(path) == file);
    }

    // Create directory
    SECTION("Create directory already exists") {
        DatPath path("directory");
        REQUIRE_FALSE(vfs->createDirectory(path) != nullptr);

        // Check the directory wasn't overwritten
        REQUIRE_FALSE(vfs->getDirectory(path)->empty());
    }

    // Is Root
    SECTION("Is root in sub-directory") {
        REQUIRE_FALSE(vfs->getDirectory("directory")->isRoot());
    }

    // Count Files
    SECTION("Count files") {
        REQUIRE(vfs->countFiles("directory") == 4);
    }

    SECTION("Count files with predicate") {
        int count = vfs->countFiles("directory", false, [](std::string name, IDvfsFile* file){
            return name.length() > 4;
        });
        REQUIRE(count == 3);
    }

    SECTION("Count files recursive") {
        REQUIRE(vfs->countFiles("directory2", true) == 8);
    }

    // Count Directories
    SECTION("Count Directories") {
        REQUIRE(vfs->countDirectories() == 2);
    }

    SECTION("Count directories with predicate") {
        int count = vfs->countDirectories("", false, [](std::string name, DatVFS* directory){
            return name.length() > 9;
        });
        REQUIRE(count == 1);
    }

    SECTION("Count files recursive") {
        REQUIRE(vfs->countDirectories("", true) == 3);
    }
}