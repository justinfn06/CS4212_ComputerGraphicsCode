#include "Sphere.h"

bool Sphere::intersect(const ray& r, float t_min, float& t_max, HitStruct& hit) const
{
    vec3 oc = r.origin() - center;

    float a = dot(r.direction(), r.direction());
    float b = 2.0f * dot(oc, r.direction());
    float c = dot(oc, oc) - radius*radius;

    float discriminant = b*b - 4*a*c;

    if (discriminant < 0) {
        return false;
    }

    float sqrt_disc = std::sqrt(discriminant);

    float t1 = (-b - sqrt_disc) / (2.0f * a);
    float t2 = (-b + sqrt_disc) / (2.0f * a);

    if (t1 > t_min && t1 < t_max) {
        t_max = t1;
        hit.t = t1;
        hit.point = r.at(t1);
        hit.shape = this;
        return true;
    }

    if (t2 > t_min && t2 < t_max) {
        t_max = t2;
        hit.t = t2;
        hit.point = r.at(t2);
        hit.shape = this;
        return true;
    }

    return false;
}

vec3 Sphere::getColor() const
{
    return color;
}