//
// Created by jacob on 30/06/23.
//
#include <catch2/catch.hpp>
#include <iostream>

#include <dat-vfs.h>

using namespace DVFS;

class MockDVFSFile : public IDVFSFile {
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

class MockDVFSFileInserter : public IDVFSFileInserter {
public:
    [[nodiscard]] std::vector<pair> getAllFiles() const override {
        return {
                {"test", new MockDVFSFile},
                {"test2", new MockDVFSFile},
                {"test3", new MockDVFSFile},
                {"test4", new MockDVFSFile},
                {"folder/test", new MockDVFSFile},
                {"folder/test2", new MockDVFSFile},
                {"folder/test3", new MockDVFSFile},
                {"folder/test4", new MockDVFSFile},
                {"folder2/test", new MockDVFSFile},
                {"folder2/test2", new MockDVFSFile},
                {"folder2/test3", new MockDVFSFile},
                {"folder2/test4", new MockDVFSFile},
                {"folder2/folder/test", new MockDVFSFile},
                {"folder2/folder/test2", new MockDVFSFile},
                {"folder2/folder/test3", new MockDVFSFile},
                {"folder2/folder/test4", new MockDVFSFile},
        };
    }
};

TEST_CASE("DatVFS Empty VFS", "[DatVFS]") {
    DatVFS* vfs = new DatVFS;

    // Create folder
    SECTION("Create single folder") {
        DatPath path("test");
        REQUIRE(vfs->createFolder(path) != nullptr);
        REQUIRE(vfs->exists(path));
    }

    SECTION("Create recursive folder") {
        DatPath path("test/recursive");
        REQUIRE(vfs->createFolder(path, true) != nullptr);
        REQUIRE(vfs->exists(path) == -1);
    }

    SECTION("Create recursive folder wrong") {
        DatPath path("test/recursive");
        REQUIRE_FALSE(vfs->createFolder(path, false) != nullptr);
        REQUIRE_FALSE(vfs->exists(path) == -1);
    }

    // Mount file
    SECTION("Mount File") {
        DatPath path("test");
        REQUIRE(vfs->mountFile(path, new MockDVFSFile));
        REQUIRE(vfs->exists(path) == 1);
    }

    SECTION("Mount file Create Folders") {
        DatPath path("test/create/folders");
        REQUIRE(vfs->mountFile(path, new MockDVFSFile, true));
        REQUIRE(vfs->exists(path) == 1);
    }

    SECTION("Mount file Create Folders wrong") {
        DatPath path("test/create/folders");
        REQUIRE_FALSE(vfs->mountFile(path, new MockDVFSFile, false));
        REQUIRE_FALSE(vfs->exists(path) == 1);
    }

    SECTION("Mount files, create folders") {
        REQUIRE(vfs->mountFiles("", MockDVFSFileInserter(), true) == 16);

        REQUIRE(vfs->exists("test"));
        REQUIRE(vfs->exists("test2"));
        REQUIRE(vfs->exists("test3"));
        REQUIRE(vfs->exists("test4"));
        REQUIRE(vfs->exists("folder/test"));
        REQUIRE(vfs->exists("folder/test2"));
        REQUIRE(vfs->exists("folder/test3"));
        REQUIRE(vfs->exists("folder/test4"));
        REQUIRE(vfs->exists("folder2/test"));
        REQUIRE(vfs->exists("folder2/test2"));
        REQUIRE(vfs->exists("folder2/test3"));
        REQUIRE(vfs->exists("folder2/test4"));
        REQUIRE(vfs->exists("folder2/folder/test"));
        REQUIRE(vfs->exists("folder2/folder/test2"));
        REQUIRE(vfs->exists("folder2/folder/test3"));
        REQUIRE(vfs->exists("folder2/folder/test4"));
    }

    SECTION("Mount files, don't create folders") {
        REQUIRE(vfs->mountFiles("", MockDVFSFileInserter(), false) == 4);

        REQUIRE(vfs->exists("test"));
        REQUIRE(vfs->exists("test2"));
        REQUIRE(vfs->exists("test3"));
        REQUIRE(vfs->exists("test4"));
        REQUIRE_FALSE(vfs->exists("folder/test"));
        REQUIRE_FALSE(vfs->exists("folder/test2"));
        REQUIRE_FALSE(vfs->exists("folder/test3"));
        REQUIRE_FALSE(vfs->exists("folder/test4"));
        REQUIRE_FALSE(vfs->exists("folder2/test"));
        REQUIRE_FALSE(vfs->exists("folder2/test2"));
        REQUIRE_FALSE(vfs->exists("folder2/test3"));
        REQUIRE_FALSE(vfs->exists("folder2/test4"));
        REQUIRE_FALSE(vfs->exists("folder2/folder/test"));
        REQUIRE_FALSE(vfs->exists("folder2/folder/test2"));
        REQUIRE_FALSE(vfs->exists("folder2/folder/test3"));
        REQUIRE_FALSE(vfs->exists("folder2/folder/test4"));
    }

    // Empty
    SECTION("Root is empty") {
        REQUIRE(vfs->empty());
    }

    SECTION("Root contains file") {
        DatPath path("test");
        REQUIRE(vfs->mountFile(path, new MockDVFSFile));
        REQUIRE_FALSE(vfs->empty());
    }

    SECTION("Root contains folder") {
        DatPath path("test");
        REQUIRE(vfs->createFolder(path));
        REQUIRE_FALSE(vfs->empty());
    }

    SECTION("Sub-folder is empty") {
        DatPath path("folder");
        vfs->createFolder(path);

        REQUIRE(vfs->empty(path));
    }


    SECTION("Sub-folder contains file") {
        DatPath path("test");
        vfs->createFolder(path);
        REQUIRE(vfs->mountFile(path/"file", new MockDVFSFile));

        REQUIRE_FALSE(vfs->empty(path));
    }

    SECTION("Sub-folder contains folder") {
        DatPath path("test");
        REQUIRE(vfs->createFolder(path/"folder", true));
        REQUIRE_FALSE(vfs->empty());
    }

    // Is Root
    SECTION("Is Root in root") {
        REQUIRE(vfs->isRoot());
    }
}

TEST_CASE("DatVFS populated VFS", "[DatVFS]") {
    DatVFS* vfs = new DatVFS;

    vfs->mountFiles("", MockDVFSFileInserter(), true);

    // Mount file
    SECTION("Mount file already exists") {
        DatPath path("test");
        IDVFSFile* file = new MockDVFSFile;
        REQUIRE_FALSE(vfs->mountFile(path, file));

        // Check file hasn't replaced old file
        REQUIRE_FALSE(vfs->getFile(path) == file);
    }

    // Mount folder
    SECTION("Create folder already exists") {
        DatPath path("folder");
        REQUIRE_FALSE(vfs->createFolder(path) != nullptr);

        // Check the folder wasn't overwritten
        REQUIRE_FALSE(vfs->getFolder(path)->empty());
    }

    // Is Root
    SECTION("Is root in sub-folder") {
        REQUIRE_FALSE(vfs->getFolder("folder")->isRoot());
    }
}