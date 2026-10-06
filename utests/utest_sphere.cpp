#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "renderlib/sphere.h"

TEST_CASE("Ray misses sphere") {
    Sphere s(point3(0, 0, -5), 1.0);
    ray r(point3(0, 2, 0), vec3(0, 0, -1));

    float t_max = 1000.0f;
    HitStruct hit;

    REQUIRE_FALSE(s.intersect(r, 0.001f, t_max, hit));
    REQUIRE(hit.shape == nullptr);
}

TEST_CASE("Ray hits sphere") {
    Sphere s(point3(0, 0, -5), 1.0);
    ray r(point3(0, 0, 0), vec3(0, 0, -1));

    float t_max = 1000.0f;
    HitStruct hit;

    REQUIRE(s.intersect(r, 0.001f, t_max, hit));
    REQUIRE_THAT(hit.t, Catch::Matchers::WithinAbs(4.0f, 1e-6f));
    REQUIRE_THAT(hit.point.x(), Catch::Matchers::WithinAbs(0.0, 1e-6));
    REQUIRE_THAT(hit.point.y(), Catch::Matchers::WithinAbs(0.0, 1e-6));
    REQUIRE_THAT(hit.point.z(), Catch::Matchers::WithinAbs(-4.0, 1e-6));
    REQUIRE(hit.shape == &s);
}

TEST_CASE("Ray starts inside sphere") {
    Sphere s(point3(0, 0, 0), 1.0);
    ray r(point3(0, 0, 0), vec3(1, 0, 0));

    float t_max = 1000.0f;
    HitStruct hit;

    REQUIRE(s.intersect(r, 0.001f, t_max, hit));
    REQUIRE_THAT(hit.t, Catch::Matchers::WithinAbs(1.0f, 1e-6f));
    REQUIRE_THAT(hit.point.x(), Catch::Matchers::WithinAbs(1.0, 1e-6));
    REQUIRE_THAT(hit.point.y(), Catch::Matchers::WithinAbs(0.0, 1e-6));
    REQUIRE_THAT(hit.point.z(), Catch::Matchers::WithinAbs(0.0, 1e-6));
    REQUIRE(hit.shape == &s);
}
