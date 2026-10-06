#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "renderlib/ray.h"
void require_vec3_near(const vec3& actual, const vec3& expected) {
    REQUIRE_THAT(actual.x(), Catch::Matchers::WithinAbs(expected.x(), 1e-6));
    REQUIRE_THAT(actual.y(), Catch::Matchers::WithinAbs(expected.y(), 1e-6));
    REQUIRE_THAT(actual.z(), Catch::Matchers::WithinAbs(expected.z(), 1e-6));
}

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
    require_vec3_near(r.origin(), origin);
    require_vec3_near(r.direction(), direction);
}

TEST_CASE("Ray evaluation \"at\" function works") {
    point3 origin(1, 2, 3);
    vec3 direction(4, 5, 6);
    ray r(origin, direction);
    require_vec3_near(r.at(0), origin);
    require_vec3_near(r.at(1), origin + direction);
    require_vec3_near(r.at(2), origin + 2 * direction);
    require_vec3_near(r.at(-1), origin - direction);
}

TEST_CASE("Ray immutability works") {
    point3 origin(1, 2, 3);
    vec3 direction(4, 5, 6);
    ray r(origin, direction);
    require_vec3_near(r.origin(), origin);
    require_vec3_near(r.direction(), direction);
}

TEST_CASE("Ray numerical robustness works") {
    point3 origin(1, 2, 3);
    vec3 direction(4, 5, 6);
    ray r(origin, direction);

    require_vec3_near(r.at(0.5), origin + 0.5 * direction);
    require_vec3_near(r.at(1.5), origin + 1.5 * direction);
}
