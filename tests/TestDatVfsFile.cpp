#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <fstream>

#include <DatVfsFile.h>
#include <iostream>

using namespace Dvfs;

TEST_CASE("LooseDvfsFile", "[IDvfsFile]") {
    SECTION("Valid File") {
        const std::filesystem::path filePath("../../include/DatVfs.h");
        IDvfsFile* dvfsFile = new LooseDvfsFile(filePath);

        REQUIRE(dvfsFile->isValidFile());

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
        IDvfsFile* dvfsFile = new LooseDvfsFile("./blatantly/bad/path");

        REQUIRE_FALSE(dvfsFile->isValidFile());

        SECTION("Filesize") {
            REQUIRE(dvfsFile->fileSize() == 0);
        }

        SECTION("Getting Content") {
            char* fileBuffer = new char[1];
            REQUIRE_FALSE(dvfsFile->getContent(fileBuffer));
        }
    }
}