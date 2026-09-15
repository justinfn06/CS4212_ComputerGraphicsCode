#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "renderlib/vec3.h"

TEST_CASE("Constructor initialized to 0, 0, 0") {
    vec3 v;
    REQUIRE(v.x() == 0);
    REQUIRE(v.y() == 0);
    REQUIRE(v.z() == 0);
}

TEST_CASE("Constructor with parameters initializes correctly") {
    vec3 v(1.0, 2.0, 3.0);
    REQUIRE(v.x() == 1.0);
    REQUIRE(v.y() == 2.0);
    REQUIRE(v.z() == 3.0);
}

TEST_CASE("Adding vectors works") {
    vec3 v1(1.0, 2.0, 3.0);
    vec3 v2(4.0, 5.0, 6.0);
    vec3 result = v1 + v2;
    REQUIRE_THAT(result.x(), Catch::Matchers::WithinAbs(5.0, 1e-6));
    REQUIRE_THAT(result.y(), Catch::Matchers::WithinAbs(7.0, 1e-6));
    REQUIRE_THAT(result.z(), Catch::Matchers::WithinAbs(9.0, 1e-6));
}

TEST_CASE("Subtracting vectors works") {
    vec3 v1(4.0, 5.0, 6.0);
    vec3 v2(1.0, 2.0, 3.0);
    vec3 result = v1 - v2;
    REQUIRE_THAT(result.x(), Catch::Matchers::WithinAbs(3.0, 1e-6));
    REQUIRE_THAT(result.y(), Catch::Matchers::WithinAbs(3.0, 1e-6));
    REQUIRE_THAT(result.z(), Catch::Matchers::WithinAbs(3.0, 1e-6));
}

TEST_CASE("Dot product works") {
    vec3 v1(1.0, 2.0, 3.0);
    vec3 v2(4.0, 5.0, 6.0);
    double result = dot(v1, v2);
    REQUIRE_THAT(result, Catch::Matchers::WithinAbs(32.0, 1e-6));
}

TEST_CASE("Cross product works") {
    vec3 v1(1.0, 0.0, 0.0);
    vec3 v2(0.0, 1.0, 0.0);
    vec3 result = cross(v1, v2);
    REQUIRE_THAT(result.x(), Catch::Matchers::WithinAbs(0.0, 1e-6));
    REQUIRE_THAT(result.y(), Catch::Matchers::WithinAbs(0.0, 1e-6));
    REQUIRE_THAT(result.z(), Catch::Matchers::WithinAbs(1.0, 1e-6));
}

TEST_CASE("Length is calculated right") {
    vec3 v(3.0, 4.0, 0.0);
    REQUIRE_THAT(v.length(), Catch::Matchers::WithinAbs(5.0, 1e-6));
}

TEST_CASE("Unit vector is calculated right") {
    vec3 v(3.0, 4.0, 0.0);
    vec3 unit = unit_vector(v);
    REQUIRE_THAT(unit.length(), Catch::Matchers::WithinAbs(1.0, 1e-6));
}