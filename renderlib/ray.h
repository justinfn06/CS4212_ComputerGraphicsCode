#ifndef RAY_H
#define RAY_H

#include "vec3.h"

// the ray class represents a ray in 3D space.
// A ray has an origin point and a direction vector.
// The ray can compute a point along the ray given a parameter t
// The ray is defined by the equation: P(t) = origin + t * direction
class ray {
    public:
    ray() = default;

    ray(const point3& origin, const vec3& direction)
        : orig(origin), dir(direction) {}

    const point3& origin() const { return orig; }
    const vec3& direction() const { return dir; }

    point3 at(double t) const {
        return orig + t * dir;
    }

    private:
    // default initialization
    point3 orig{0.0, 0.0, 0.0};
    vec3 dir{0.0, 0.0, 0.0};
};

// used only for testing
inline bool operator==(const ray& a, const ray& b) {
    return a.origin() == b.origin() && a.direction() == b.direction();
}

// used only for testing
inline bool operator!=(const ray& a, const ray& b) {
    return !(a == b);
}

#endif