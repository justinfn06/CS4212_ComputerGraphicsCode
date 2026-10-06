#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "renderlib/ray.h"

TEST_CASE("Constructor initialized to 0, 0, 0") {
    vec3 v;
    REQUIRE(v.x() == 0);
    REQUIRE(v.y() == 0);
    REQUIRE(v.z() == 0);
}

TEST_CASE("Ray construction works") {
    point3 origin(1, 2, 3);
    vec3 direction(4, 5, 6);
    ray r(origin, direction);
    REQUIRE(r.origin() == origin);
    REQUIRE(r.direction() == direction);
}

TEST_CASE("Ray evaluation \"at\" function works") {
    point3 origin(1, 2, 3);
    vec3 direction(4, 5, 6);
    ray r(origin, direction);
    REQUIRE(r.at(0) == origin);
    REQUIRE(r.at(1) == origin + direction);
    REQUIRE(r.at(2) == origin + 2 * direction);
    REQUIRE(r.at(-1) == origin - direction);
}

TEST_CASE("Ray immutability works") {
    point3 origin(1, 2, 3);
    vec3 direction(4, 5, 6);
    ray r(origin, direction);
    REQUIRE(r.origin() == origin);
    REQUIRE(r.direction() == direction);
}

TEST_CASE("Ray numerical robustness works") {
    point3 origin(1, 2, 3);
    vec3 direction(4, 5, 6);
    ray r(origin, direction);
    REQUIRE(r.at(0.5) == origin + 0.5 * direction);
}
