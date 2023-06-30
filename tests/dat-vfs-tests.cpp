//
// Created by jacob on 30/06/23.
//
#include <catch2/catch.hpp>
#include <iostream>

#include <dat-vfs.h>

using namespace DVFS;

TEST_CASE("DatVFS Empty VFS", "[DatVFS]") {
    DatVFS* vfs = new DatVFS;

    // Create folder
    // Mount file
    //
}

TEST_CASE("DatVFS populated VFS", "[DatVFS]") {
    DatVFS* vfs = new DatVFS;

    vfs->mountFiles("", DVFSLooseFileInserter("/home/jacob/Documents"), true);

    std::cout << vfs->tree();

    REQUIRE(true);
}