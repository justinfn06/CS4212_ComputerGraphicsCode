#ifndef SPHERE_H
#define SPHERE_H

#include "shape.h"
#include "vec3.h"

class Sphere : public Shape {
public:
    Sphere(const point3& center, double radius)
        : center_(center), radius_(radius) {}

    bool intersect(const ray& r) const override {
        vec3 oc = r.origin() - center_;

        double a = dot(r.direction(), r.direction());
        double b = 2.0 * dot(oc, r.direction());
        double c = dot(oc, oc) - radius_ * radius_;

        double discriminant = b * b - 4.0 * a * c;

        return discriminant >= 0.0;
    }

private:
    point3 center_;
    double radius_;
};

#endif