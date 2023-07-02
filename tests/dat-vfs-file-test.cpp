//
// Created by jacob on 01/07/23.
//

#include <catch2/catch.hpp>
#include <cstring>
#include <fstream>

#include <dat-vfs-file.h>

using namespace DVFS;

TEST_CASE("LooseDVFSFile", "[IDVFSFile]") {
    SECTION("Valid File") {
        const std::filesystem::path filePath("./include/dat-vfs.h");
        IDVFSFile* dvfsFile = new LooseDVFSFile(filePath);

        SECTION("Validation") {
            REQUIRE(dvfsFile->isValidFile());
        }

        SECTION("Filesize") {
            REQUIRE(dvfsFile->fileSize() == std::filesystem::file_size(filePath));
        }

        SECTION("Getting Content") {
            char* fileBuffer = new char[dvfsFile->fileSize()];
            REQUIRE(dvfsFile->getContent(fileBuffer));

            std::ifstream stream(filePath, std::ios::binary | std::ios::in | std::ios::ate);
            std::streamsize actualSize = stream.tellg();

            REQUIRE(dvfsFile->fileSize() == actualSize);

            stream.seekg(0);
            char* actualFileBuffer = new char[actualSize];
            stream.read(actualFileBuffer, actualSize);
            stream.close();

            REQUIRE(strncmp(fileBuffer, actualFileBuffer, actualSize) == 0);
            delete[] actualFileBuffer;
            delete[] fileBuffer;
        }
    }

    SECTION("Invalid File") {
        IDVFSFile* dvfsFile = new LooseDVFSFile("./blatantly/bad/path");

        SECTION("Validation") {
            REQUIRE_FALSE(dvfsFile->isValidFile());
        }

        SECTION("Filesize") {
            REQUIRE(dvfsFile->fileSize() == 0);
        }

        SECTION("Getting Content") {
            char* fileBuffer = new char[1];
            REQUIRE_FALSE(dvfsFile->getContent(fileBuffer));
        }
    }
}