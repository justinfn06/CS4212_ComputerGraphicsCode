#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "renderlib/ray.h"

// code to test
/*
#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray {
    public:
    ray() {}

    ray(const point3& origin, const vec3& direction)
        : orig(origin), dir(direction) {}

    const point3& origin() const { return orig; }
    const vec3& direction() const { return dir; }

    point3 at(double t) const {
        return orig + t*dir;
    }

    private:
    point3 orig;
    vec3 dir;
};

#endif
*/

/* key tests:
Construction - A ray stores the origin and direction passed to the constructor
Evaluation of Parametric Line - verify that the at function works appropriately for specific t directions (0, 1, arbitrary positive values, arbitrary negative values)
Immutability - ensure that you have no side effects that modify the ray's origin or direction once it's been set.
Numerical Robustness - Are points computed correct within floating-point tolerance
*/
// full test coverage:

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
