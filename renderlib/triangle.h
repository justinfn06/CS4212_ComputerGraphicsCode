#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "shape.h"
#include "vec3.h"

class Triangle : public Shape {
public:
    Triangle(const point3& point_a, const point3& point_b, const point3& point_c)
        : a_(point_a), b_(point_b), c_(point_c) {}

    bool intersect(const ray& r) const override {
        vec3 ab = b_ - a_;
        vec3 ac = c_ - a_;
        vec3 p = cross(r.direction(), ac);

        // the determinant is used to check if the ray is parallel to the triangle
        // if the determinant is 0, ray is parallel to triangle and there is no intersection
        // if the determinant is not 0, it is used to find the intersection.
        double det = dot(ab, p);

        if (det == 0) {
            return false; // ray is parallel to the triangle
        }


        // if the determinant is not 0, it is used to find the intersection.
        double inv_det = 1.0 / det;
        vec3 ao = r.origin() - a_;
        double u = dot(ao, p) * inv_det;
        if (u < 0 || u > 1) {
            return false;
        }
        
        // this block checks if the intersection point is inside the triangle.
        // if it is inside, that is important to know because the intersection point is only valid if it is inside the triangle.
        vec3 q = cross(ao, ab);
        double v = dot(r.direction(), q) * inv_det;
        if (v < 0 || u + v > 1) {
            return false;
        }

        return true;
    }

private:
    point3 a_;
    point3 b_;
    point3 c_;
};


#endif