#ifndef SHAPE_H
#define SHAPE_H

#include "ray.h"

class Shape {
public:
    virtual ~Shape() = default;

    virtual bool intersect(const ray& r) const = 0;
};

#endif