//
// Created by jacob on 29/06/23.
//

#include <catch2/catch.hpp>

#include <dat-path.h>

TEST_CASE("DatPath initialisation", "[DatPath]") {
    SECTION("Empty String Initialisation") {
        REQUIRE_NOTHROW(DatPath(""));
    }

    SECTION("Correctly formats path") {
        SECTION("Formatting with leading slash") {
            DatPath testPath("/test/path/with/leading/slash");

            REQUIRE((std::string) testPath == "test/path/with/leading/slash");
        }


        SECTION("Formatting with many leading slashes") {
            DatPath testPath("/////test/path/with/many/leading/slashes");

            REQUIRE((std::string) testPath == "test/path/with/many/leading/slashes");
        }

        SECTION("Formatting with trailing slash") {
            DatPath testPath("test/path/with/trailing/slash/");

            REQUIRE((std::string) testPath == "test/path/with/trailing/slash");
        }

        SECTION("Formatting with many trailing slashes") {
            DatPath testPath("test/path/with/many/trailing/slashes//////");

            REQUIRE((std::string) testPath == "test/path/with/many/trailing/slashes");
        }

        SECTION("Formatting with leading and trailing slash") {
            DatPath testPath("test/path/with/trailing/slash/");

            REQUIRE((std::string) testPath == "test/path/with/trailing/slash");
        }

        SECTION("Formatting with many leading and trailing slashes") {
            DatPath testPath("///////test/path/with/many/trailing/slashes//////");

            REQUIRE((std::string) testPath == "test/path/with/many/trailing/slashes");
        }
    }
}

TEST_CASE("DatPath correct depth", "[DatPath]") {
    SECTION("Depth 3") {
        DatPath testPath("depth/three/please");
        REQUIRE(testPath.depth() == 3);
    }

    SECTION("Depth 2") {
        DatPath testPath("depth/two");
        REQUIRE(testPath.depth() == 2);
    }

    SECTION("Depth 1") {
        DatPath testPath("depth");
        REQUIRE(testPath.depth() == 1);
    }

    SECTION("Depth 0") {
        DatPath testPath("");
        REQUIRE(testPath.depth() == 0);
    }
}

TEST_CASE("DatPath correctly increments", "[DatPath]") {
    DatPath testPath("test/path/testing/navigation");

    SECTION("Increment none") {
        DatPath newPath = testPath.increment(0);
        REQUIRE(newPath.depth() == 4);
        REQUIRE((std::string) newPath == "test/path/testing/navigation");
    }

    SECTION("Increment 1") {
        DatPath newPath = testPath.increment();
        REQUIRE(newPath.depth() == 3);
        REQUIRE((std::string) newPath == "path/testing/navigation");
    }

    SECTION("Increment 2") {
        DatPath newPath = testPath.increment(2);
        REQUIRE(newPath.depth() == 2);
        REQUIRE((std::string) newPath == "testing/navigation");
    }

    SECTION("Increment 3") {
        DatPath newPath = testPath.increment(3);
        REQUIRE(newPath.depth() == 1);
        REQUIRE((std::string) newPath == "navigation");
    }

    SECTION("Increment to end") {
        DatPath newPath = testPath.increment(4);
        REQUIRE(newPath.depth() == 0);
        REQUIRE(newPath.empty());
    }

    SECTION("Increment passed end") {
        DatPath newPath = testPath.increment(10);
        REQUIRE(newPath.depth() == 0);
        REQUIRE(newPath.empty());
    }
}

TEST_CASE("DatPath Get Root", "[DatPath]") {
    SECTION("Empty path") {
        DatPath path;
        REQUIRE(path.getRoot() == DatPath(""));
    }

    SECTION("Depth 1 path") {
        DatPath path("test");
        REQUIRE(path.getRoot() == DatPath("test"));
    }

    SECTION("Depth 2 path") {
        DatPath path("test/test2");
        REQUIRE(path.getRoot() == DatPath("test"));
    }

    SECTION("Depth 3 path") {
        DatPath path("test/test2/test3");
        REQUIRE(path.getRoot() == DatPath("test"));
    }
}

TEST_CASE("DatPath empty") {
    SECTION("Not Empty") {
        DatPath path("not/empty");
        REQUIRE_FALSE(path.empty());
    }
    SECTION("Empty") {
        DatPath path;
        REQUIRE(path.empty());
    }
}