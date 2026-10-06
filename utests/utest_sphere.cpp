#include <catch2/catch_test_macros.hpp>
#include "renderlib/sphere.h"

TEST_CASE("Ray misses sphere") {
    Sphere s(point3(0, 0, -5), 1.0);
    ray r(point3(0, 2, 0), vec3(0, 0, -1));
    REQUIRE_FALSE(s.intersect(r));
}

TEST_CASE("Ray hits sphere") {
    Sphere s(point3(0, 0, -5), 1.0);
    ray r(point3(0, 0, 0), vec3(0, 0, -1));
    REQUIRE(s.intersect(r));
}

TEST_CASE("Ray starts inside sphere") {
    Sphere s(point3(0, 0, 0), 1.0);
    ray r(point3(0, 0, 0), vec3(1, 0, 0));
    REQUIRE(s.intersect(r));
}